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

Decider::Decider(std::unique_ptr<Interface> && interface, std::shared_ptr<Plugin> plugin)
{
  interface_ = std::move(interface);
  plugin_ = plugin;

  driving_mode_config_ = std::make_unique<DrivingModeConfig>();
  plugin_->setup(*driving_mode_config_);
  driving_mode_status_ =
    std::make_unique<SystemModeStatusStore>(driving_mode_config_->autoware_modes());

  current_modes_.operation_autoware_mode =
    driving_mode_config_->from_operation_mode(OperationMode::kStop);
  current_modes_.autoware_control = AutowareControl::kUnknown;
  current_modes_.mrm_request = MrmRequest::kNone;
  current_modes_.autoware_mode = AutowareMode{0};  // unknown mode
}

SystemModeStatusStore & Decider::access_status()
{
  return *driving_mode_status_;
}

void Decider::update()
{
  // RCLCPP_INFO_STREAM(logger, "Current Autoware Mode: " << current_modes_.autoware_mode.id);
  // print_modes("Temporary Unavailable Modes", temporary_unavailable_modes_);

  // Detect status timeout.
  driving_mode_status_->update(interface_->now(), 1.0);

  // List available modes.
  AutowareModeSet availables;
  for (const auto & mode : driving_mode_config_->autoware_modes()) {
    if (temporary_unavailable_modes_.count(mode) == 0) {
      if (mode.id != current_modes_.autoware_mode.id) {
        if (driving_mode_status_->is_available(mode)) availables.insert(mode);
      } else {
        if (driving_mode_status_->is_continuable(mode)) availables.insert(mode);
      }
    }
  }

  // TODO(isamu-takagi): Check frequently mode change.
  update_autoware_mode(plugin_->decide(current_modes_, availables));
  execute_tasks();
}

void Decider::execute_tasks()
{
  while (!tasks_.empty()) {
    const auto result = tasks_.front()->execute(*interface_, gates_);
    switch (result) {
      case TaskResult::kFinished:
        tasks_.pop();
        break;
      case TaskResult::kTimeout:
        tasks_ = std::queue<std::unique_ptr<Task>>();
        temporary_unavailable_modes_.insert(current_modes_.autoware_mode);
        return;
      case TaskResult::kRunning:
        return;
      default:
        throw std::logic_error("invalid task result");
    }
  }
}

void Decider::notify_trajectory_source(const TrajectorySource & source)
{
  if (gates_.expect.trajectory_source != source) {
    // Unintended change detected !!
  }
  gates_.status.trajectory_source = source;
  gates_.expect.trajectory_source = source;
}

void Decider::notify_command_source(const CommandSource & source)
{
  if (gates_.expect.command_source != source) {
    // Unintended change detected !!
  }
  gates_.status.command_source = source;
  gates_.expect.command_source = source;
}

void Decider::notify_vehicle_control_mode(const PlatformMode & mode)
{
  if (gates_.expect.platform_mode != mode) {
    // Override detected !!
  }
  gates_.status.platform_mode = mode;
  gates_.expect.platform_mode = mode;
}

void Decider::update_autoware_mode(const AutowareMode & mode)
{
  AutowareMode & prev = current_modes_.autoware_mode;
  if (prev.id == mode.id) {
    return;
  }
  if (!driving_mode_config_->exists(mode)) {
    RCLCPP_ERROR_STREAM(logger, "decision logic returns unknown mode: " << mode.id);
    return;
  }

  RCLCPP_INFO_STREAM(logger, "Change Autoware Mode: " << prev.id << " -> " << mode.id);
  prev = mode;

  const auto gates = driving_mode_config_->gates(mode);
  std::queue<std::unique_ptr<Task>> tasks;
  if (gates.trajectory_source) {
    tasks.push(std::make_unique<TrajectorySourceTask>(*gates.trajectory_source));
  }
  if (gates.command_source) {
    tasks.push(std::make_unique<CommandSourceTask>(*gates.command_source));
  }
  tasks_.swap(tasks);
}

void Decider::update_platform_mode(const PlatformMode & mode)
{
  (void)mode;
}

void Decider::change_operation_mode(const OperationMode & operation_mode)
{
  const auto mode = driving_mode_config_->from_operation_mode(operation_mode);

  // TODO(isamu-takagi): Implement background mode change.
  // if (current_modes_.autoware_control == AutowareControl::kDisable) {
  // }

  if (driving_mode_status_->is_available(mode)) {
    RCLCPP_INFO_STREAM(logger, "change operation mode: " << mode.id);
    current_modes_.operation_autoware_mode = mode;
  } else {
    RCLCPP_WARN_STREAM(logger, "reject operation mode: " << mode.id);
  }
}

void Decider::change_autoware_control(const AutowareControl & autoware_control)
{
  if (autoware_control == AutowareControl::kDisable) {
    return;
  }

  if (autoware_control == AutowareControl::kEnable) {
    return;
  }

  // The check target is the normal behavior, operation mode. MRM is not included.
  if (!driving_mode_status_->is_available(current_modes_.operation_autoware_mode)) {
    RCLCPP_WARN_STREAM(logger, "reject autoware control change");
  }

  RCLCPP_INFO_STREAM(logger, "accept autoware control change");
  current_modes_.autoware_control = autoware_control;
}

}  // namespace autoware::system_mode_decider
