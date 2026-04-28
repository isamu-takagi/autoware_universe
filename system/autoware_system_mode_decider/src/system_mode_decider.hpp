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

#include "core/decider.hpp"

#include <autoware_system_mode_decider/plugin.hpp>
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <pluginlib/class_loader.hpp>

#include <autoware_system_mode_msgs/msg/system_mode_status.hpp>
#include <autoware_system_mode_msgs/msg/trajectory_source.hpp>
#include <autoware_system_msgs/srv/change_autoware_control.hpp>
#include <autoware_system_msgs/srv/change_operation_mode.hpp>
#include <autoware_vehicle_msgs/msg/control_mode_report.hpp>
#include <autoware_vehicle_msgs/srv/control_mode_command.hpp>
#include <tier4_system_msgs/msg/command_source_status.hpp>
#include <tier4_system_msgs/srv/select_command_source.hpp>

#include <memory>

namespace autoware::system_mode_decider
{

class SystemModeDecider : public rclcpp::Node
{
public:
  explicit SystemModeDecider(const rclcpp::NodeOptions & options);

private:
  using SystemModeStatus = autoware_system_mode_msgs::msg::SystemModeStatus;
  using TrajectorySourceMsg = autoware_system_mode_msgs::msg::TrajectorySource;
  using CommandSourceMsg = tier4_system_msgs::msg::CommandSourceStatus;
  using ControlModeReport = autoware_vehicle_msgs::msg::ControlModeReport;
  using ChangeOperationMode = autoware_system_msgs::srv::ChangeOperationMode;
  using ChangeAutowareControl = autoware_system_msgs::srv::ChangeAutowareControl;

  diagnostic_updater::Updater diag_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<SystemModeStatus>::SharedPtr sub_system_mode_status_;
  rclcpp::Subscription<TrajectorySourceMsg>::SharedPtr sub_trajectory_source_;
  rclcpp::Subscription<CommandSourceMsg>::SharedPtr sub_command_source_;
  rclcpp::Subscription<ControlModeReport>::SharedPtr sub_control_mode_report_;
  rclcpp::Service<ChangeOperationMode>::SharedPtr srv_operation_mode_;
  rclcpp::Service<ChangeAutowareControl>::SharedPtr srv_autoware_control_;
  void on_timer_init();
  void on_timer_main();

  void on_system_mode_status(const SystemModeStatus & msg);
  void on_trajectory_source(const TrajectorySourceMsg & msg);
  void on_command_source(const CommandSourceMsg & msg);
  void on_control_mode_report(const ControlModeReport & msg);
  void on_change_operation_mode(
    ChangeOperationMode::Request::SharedPtr req, ChangeOperationMode::Response::SharedPtr res);
  void on_change_autoware_control(
    ChangeAutowareControl::Request::SharedPtr req, ChangeAutowareControl::Response::SharedPtr res);

  pluginlib::ClassLoader<Plugin> loader_;
  std::unique_ptr<Decider> decider_;

  // For initialization.
  int init_flag_ = 0;
};

class RosInterface : public Interface
{
public:
  explicit RosInterface(rclcpp::Node * node);
  rclcpp::Time now() const override;
  void change_trajectory_source(const TrajectorySource & source) override;
  void change_command_source(const CommandSource & source) override;
  void change_platform_mode(const PlatformMode & mode) override;

private:
  using TrajectorySourceMsg = autoware_system_mode_msgs::msg::TrajectorySource;
  using SelectCommandSourceSrv = tier4_system_msgs::srv::SelectCommandSource;
  using ControlModeCommandSrv = autoware_vehicle_msgs::srv::ControlModeCommand;
  rclcpp::Node * node_;
  rclcpp::Publisher<TrajectorySourceMsg>::SharedPtr pub_trajectory_select_;
  rclcpp::Client<SelectCommandSourceSrv>::SharedPtr cli_command_select_;
  rclcpp::Client<ControlModeCommandSrv>::SharedPtr cli_control_mode_command_;
};

}  // namespace autoware::system_mode_decider

#endif  // SYSTEM_MODE_DECIDER_HPP_
