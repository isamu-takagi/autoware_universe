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
#include <utility>
#include <vector>

namespace autoware::system_mode_decider
{

const auto logger = rclcpp::get_logger("Decider");

std::vector<AutowareMode> modes_from_mapping(const ModeMapping & mapping)
{
  std::vector<AutowareMode> modes;
  for (const auto & [mode_id, _] : mapping) {
    modes.emplace_back(AutowareMode{mode_id});
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
  // Detect status timeout.
  driving_mode_status_.update(interface_->now(), 1.0);

  // TODO(isamu-takagi): Check frequently mode change.
  update_autoware_mode(plugin_->decide(current_modes_, driving_mode_status_));

  Task & task = tasks_.empty() ? none_task_ : tasks_.front();
  task.execute(*interface_);
  if (task.timeout(*interface_)) {
    tasks_.pop();
    RCLCPP_INFO_STREAM(logger, "timeout");
  }
}

void Decider::notify_gate_status(const GateStatus & status)
{
  actual_gate_status_[status.type] = status.id;

  Task & task = tasks_.empty() ? none_task_ : tasks_.front();

  if (task.expects(status)) {
    tasks_.pop();
    // update();
    RCLCPP_INFO_STREAM(logger, "complete");
  } else {
    // Override detected !!
  }
}

void Decider::update_autoware_mode(const AutowareMode & mode)
{
  AutowareMode & prev = current_modes_.autoware_mode;
  if (mapping_.count(mode.id) == 0) {
    RCLCPP_ERROR_STREAM(logger, "decision logic returns unknown mode: " << mode.id);
    return;
  }
  if (prev.id == mode.id) return;
  RCLCPP_INFO_STREAM(logger, "Change Autoware Mode: " << prev.id << " -> " << mode.id);
  prev = mode;

  std::queue<Task> tasks;
  for (const auto & status : mapping_.at(mode.id)) {
    tasks.push(Task(status));
  }
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
  RCLCPP_INFO_STREAM(logger, "Change Operation Mode: " << mode.id);

  current_modes_.operation_mode = operation_mode;
}

}  // namespace autoware::system_mode_decider
