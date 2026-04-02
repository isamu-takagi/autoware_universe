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

#ifndef GATE__TRAJECTORY_HPP_
#define GATE__TRAJECTORY_HPP_

#include <autoware_system_mode_msgs/msg/trajectory_source.hpp>

namespace autoware::system_mode_decider
{

class TrajectoryGate
{
public:
  TrajectoryGate() = default;

private:
  using TrajectorySource = autoware_system_mode_msgs::msg::TrajectorySource;
  rclcpp::Subscription<TrajectorySource>::SharedPtr sub_source_;
  rclcpp::Publisher<TrajectorySource>::SharedPtr pub_source_;
};

}  // namespace autoware::system_mode_decider

#endif  // GATE__TRAJECTORY_HPP_
