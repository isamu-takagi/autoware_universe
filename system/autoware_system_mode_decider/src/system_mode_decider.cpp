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

#include "system_mode_decider.hpp"

namespace autoware::system_mode_decider
{

SystemModeDecider::SystemModeDecider(const rclcpp::NodeOptions & options)
: Node("system_mode_decider", options), diag_(this, 0.1)
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  diag_.setHardwareID("none");

  sub_trajectory_source_ = create_subscription<TrajectorySource>(
    "~/trajectory/source", rclcpp::QoS(1).transient_local(),
    std::bind(&SystemModeDecider::on_trajectory_source, this, _1));
  sub_command_source_ = create_subscription<CommandSource>(
    "~/command/source", rclcpp::QoS(1).transient_local(),
    std::bind(&SystemModeDecider::on_command_source, this, _1));
  sub_vehicle_source_ = create_subscription<VehicleSource>(
    "~/vehicle/source", rclcpp::QoS(1).volatile(),
    std::bind(&SystemModeDecider::on_vehicle_source, this, _1));

  const auto period = rclcpp::Rate(1.0).period();
  timer_ = rclcpp::create_timer(this, get_clock(), period, [this]() { on_timer_init(); });
}

void SystemModeDecider::on_timer_init()
{
}

void SystemModeDecider::on_trajectory_source(const TrajectorySource & msg)
{
  RCLCPP_INFO_STREAM(get_logger(), "on_trajectory_source: " << msg.source);
}

void SystemModeDecider::on_command_source(const CommandSource & msg)
{
  RCLCPP_INFO_STREAM(get_logger(), "on_command_source: " << msg.source);
}

void SystemModeDecider::on_vehicle_source(const VehicleSource & msg)
{
  RCLCPP_INFO_STREAM(get_logger(), "on_vehicle_source: " << static_cast<int>(msg.mode));
}

}  // namespace autoware::system_mode_decider

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::system_mode_decider::SystemModeDecider)
