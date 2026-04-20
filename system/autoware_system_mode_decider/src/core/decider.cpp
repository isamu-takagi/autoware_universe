// Copyright 2026 The Autoware Contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "decider.hpp"

#include <rclcpp/logging.hpp>

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace autoware::system_mode_decider
{

const auto logger = rclcpp::get_logger("Decider");

template <typename ModeIterable>
void print_modes(const std::string & title, const ModeIterable & modes)
{
  std::string text;
  for (const auto & mode : modes) {
    text = text + " " + std::to_string(mode.id);
  }
  RCLCPP_INFO_STREAM(logger, title << ":" << text);
}

std::vector<AutowareMode> modes_from_mapping(const ModeMapping & mapping)
{
  std::vector<AutowareMode> modes;
  for (const auto & [mode, gate] : mapping) {
    modes.emplace_back(mode);
  }
  return modes;
}

Decider::Decider(std::unique_ptr<Interface> && interface, std::shared_ptr<Plugin> plugin)
: mapping_(plugin->mapping()), driving_mode_status_(modes_from_mapping(mapping_))
{
  interface_ = std::move(interface);
  plugin_ = plugin;

  current_modes_.operation_mode = OperationMode::kStop;
  current_modes_.autoware_control = AutowareControl::kUnknown;
  current_modes_.mrm_request = MrmRequest::kNone;
  current_modes_.autoware_mode = AutowareMode{0};  // unknown mode
  current_modes_.platform_mode = PlatformMode{0};  // unknown mode
}

SystemModeStatusStore & Decider::access_status()
{
  return driving_mode_status_;
}

void Decider::update()
{
  // RCLCPP_INFO_STREAM(logger, "Current Autoware Mode: " << current_modes_.autoware_mode.id);
  // print_modes("Temporary Unavailable Modes", temporary_unavailable_modes_);

  // Detect status timeout.
  driving_mode_status_.update(interface_->now(), 1.0);

  // List available modes.
  AutowareModeSet availables;
  for (const auto & [mode, gate] : mapping_) {
    if (temporary_unavailable_modes_.count(mode) == 0) {
      if (mode.id != current_modes_.autoware_mode.id) {
        if (driving_mode_status_.is_available(mode)) availables.insert(mode);
      } else {
        if (driving_mode_status_.is_continuable(mode)) availables.insert(mode);
      }
    }
  }

  // TODO(isamu-takagi): Check frequently mode change.
  update_autoware_mode(plugin_->decide(current_modes_, availables));

  Task * task = tasks_.empty() ? none_task_.get() : tasks_.front().get();
  task->execute(*interface_);
  if (task->timeout(*interface_)) {
    tasks_ = std::queue<std::unique_ptr<Task>>();
    RCLCPP_INFO_STREAM(logger, "timeout");
    temporary_unavailable_modes_.insert(current_modes_.autoware_mode);
  }
}

void Decider::notify_gate_status(const GateStatus & status)
{
  actual_gate_status_[status.type] = status.id;

  Task * task = tasks_.empty() ? none_task_.get() : tasks_.front().get();

  if (task->expects(status)) {
    tasks_.pop();
    // update();
  } else {
    // Override detected !!
  }
}

void Decider::update_autoware_mode(const AutowareMode & mode)
{
  AutowareMode & prev = current_modes_.autoware_mode;
  if (prev.id == mode.id) {
    return;
  }
  if (mapping_.count(mode) == 0) {
    RCLCPP_ERROR_STREAM(logger, "decision logic returns unknown mode: " << mode.id);
    return;
  }
  RCLCPP_INFO_STREAM(logger, "Change Autoware Mode: " << prev.id << " -> " << mode.id);

  std::queue<std::unique_ptr<Task>> tasks;
  for (const auto & status : mapping_.at(mode)) {
    tasks.push(std::make_unique<GateTask>(status));
  }

  prev = mode;
  tasks_.swap(tasks);
}

void Decider::update_platform_mode(const PlatformMode & mode)
{
  PlatformMode & prev = current_modes_.platform_mode;

  if (prev.id == mode.id) return;
  prev = mode;
  RCLCPP_INFO_STREAM(logger, "Change Platform Mode: " << prev.id << " -> " << mode.id);
}

void Decider::change_operation_mode(const OperationMode & operation_mode)
{
  const auto mode = plugin_->from_operation_mode(operation_mode);

  if (driving_mode_status_.is_available(mode)) {
    RCLCPP_INFO_STREAM(logger, "change operation mode: " << mode.id);
    current_modes_.operation_mode = operation_mode;
  } else {
    RCLCPP_WARN_STREAM(logger, "reject operation mode: " << mode.id);
  }
}

}  // namespace autoware::system_mode_decider
