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

#include <memory>
#include <utility>

//
#include <rclcpp/logging.hpp>

namespace autoware::system_mode_decider
{

Decider::Decider(std::unique_ptr<Interface> && interface, std::shared_ptr<Plugin> plugin)
{
  interface_ = std::move(interface);
  plugin_ = plugin;
  mapping_ = plugin_->mapping();
}

void Decider::update()
{
  // std::printf("%d %d %d\n", actual_.trajectory, actual_.command, actual_.vehicle);

  Task & task = tasks_.empty() ? none_task_ : tasks_.front();
  task.execute(*interface_);
  if (task.timeout(*interface_)) {
    tasks_.pop();
    RCLCPP_INFO_STREAM(rclcpp::get_logger("Decider"), "timeout");
  }
}

void Decider::notify_gate_status(const GateStatus & status)
{
  actual_gate_status_[status.type] = status.id;

  Task & task = tasks_.empty() ? none_task_ : tasks_.front();

  if (task.expects(status)) {
    tasks_.pop();
    // update();
    RCLCPP_INFO_STREAM(rclcpp::get_logger("Decider"), "complete");
  } else {
    // Override detected !!
  }
}

void Decider::change_autoware_mode(const AutowareMode & mode)
{
  RCLCPP_INFO(rclcpp::get_logger("Decider"), "Change Autoware mode to %d", mode.id);

  for (const auto & status : mapping_.at(mode.id)) {
    tasks_.push(Task(status));
  }
}

void Decider::change_platform_mode(const PlatformMode & mode)
{
  RCLCPP_INFO(rclcpp::get_logger("Decider"), "Change Platform mode to %d", mode.id);
}

}  // namespace autoware::system_mode_decider
