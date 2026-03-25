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

#include "type/types.hpp"

#include <diagnostic_updater/diagnostic_updater.hpp>

#include <autoware_system_mode_msgs/msg/trajectory_source.hpp>
#include <autoware_vehicle_msgs/msg/control_mode_report.hpp>
#include <tier4_system_msgs/msg/command_source_status.hpp>

namespace autoware::system_mode_decider
{

class SystemModeDecider : public rclcpp::Node
{
public:
  explicit SystemModeDecider(const rclcpp::NodeOptions & options);

private:
  using TrajectorySource = autoware_system_mode_msgs::msg::TrajectorySource;
  using CommandSource = tier4_system_msgs::msg::CommandSourceStatus;
  using VehicleSource = autoware_vehicle_msgs::msg::ControlModeReport;

  diagnostic_updater::Updater diag_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<TrajectorySource>::SharedPtr sub_trajectory_source_;
  rclcpp::Subscription<CommandSource>::SharedPtr sub_command_source_;
  rclcpp::Subscription<VehicleSource>::SharedPtr sub_vehicle_source_;

  void on_timer_init();
  void on_timer_main();

  void on_trajectory_source(const TrajectorySource & msg);
  void on_command_source(const CommandSource & msg);
  void on_vehicle_source(const VehicleSource & msg);
};

}  // namespace autoware::system_mode_decider

#endif  // SYSTEM_MODE_DECIDER_HPP_
