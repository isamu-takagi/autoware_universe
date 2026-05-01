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

#ifndef ROS_INTERFACE_HPP_
#define ROS_INTERFACE_HPP_

#include "type/interface.hpp"

namespace autoware::driving_mode_manager
{

class RosInterface : public Interface
{
public:
  explicit RosInterface(rclcpp::Node * node);
  rclcpp::Time now() const override;
  void change_trajectory_source(const TrajectorySource & source) override;
  void change_command_source(const CommandSource & source) override;
  void change_platform_mode(const PlatformMode & mode) override;
  void publish_operation_mode(const OperationModeState & state) const override;

private:
  using TrajectorySourceSrv = autoware_driving_mode_msgs::srv::ChangeTrajectorySource;
  using ChangeCommandSourceSrv = autoware_driving_mode_msgs::srv::ChangeCommandSource;
  using ControlModeCommandSrv = autoware_vehicle_msgs::srv::ControlModeCommand;
  using OperationModeStateMsg = autoware_adapi_v1_msgs::msg::OperationModeState;
  rclcpp::Node * node_;
  rclcpp::Client<TrajectorySourceSrv>::SharedPtr cli_trajectory_source_;
  rclcpp::Client<ChangeCommandSourceSrv>::SharedPtr cli_command_source_;
  rclcpp::Client<ControlModeCommandSrv>::SharedPtr cli_control_mode_command_;
  rclcpp::Publisher<OperationModeStateMsg>::SharedPtr pub_operation_mode_;
};

}  // namespace autoware::driving_mode_manager

#endif  // ROS_INTERFACE_HPP_
