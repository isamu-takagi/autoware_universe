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

#ifndef SYSTEM_MODE_DECIDER_HPP_
#define SYSTEM_MODE_DECIDER_HPP_

#include <diagnostic_updater/diagnostic_updater.hpp>

#include <std_msgs/msg/u_int32.hpp>

namespace autoware::system_mode_decider
{

class SystemModeDecider : public rclcpp::Node
{
public:
  explicit SystemModeDecider(const rclcpp::NodeOptions & options);

private:
  using TrajectorySource = std_msgs::msg::UInt32;

  diagnostic_updater::Updater diag_;
};

}  // namespace autoware::system_mode_decider

#endif  // SYSTEM_MODE_DECIDER_HPP_
