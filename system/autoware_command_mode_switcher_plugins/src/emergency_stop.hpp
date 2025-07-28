
//  Copyright 2025 The Autoware Contributors
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,

//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef EMERGENCY_STOP_HPP_
#define EMERGENCY_STOP_HPP_

#include <autoware_command_mode_switcher/command_plugin.hpp>
#include <autoware_command_mode_types/modes.hpp>
#include <autoware_command_mode_types/sources.hpp>

#include <tier4_system_msgs/msg/mrm_behavior_status.hpp>
#include <tier4_system_msgs/srv/operate_mrm.hpp>

namespace autoware::command_mode_switcher
{

class EmergencyStopSwitcher : public CommandPlugin
{
public:
  uint16_t mode() const override { return autoware::command_mode_types::modes::emergency_stop; }
  uint16_t source() const override { return autoware::command_mode_types::sources::emergency_stop; }
  bool autoware_control() const override { return true; }
  void initialize() override;

  SourceState update_source_state(bool request) override;
  MrmState update_mrm_state() override;

private:
  using OperateMrmSrv = tier4_system_msgs::srv::OperateMrm;
  using MrmBehaviorStatus = tier4_system_msgs::msg::MrmBehaviorStatus;
  rclcpp::Client<OperateMrmSrv>::SharedPtr cli_operate_;
  rclcpp::Subscription<MrmBehaviorStatus>::SharedPtr sub_status_;

  void operate(bool request);
  bool requesting_;
  MrmBehaviorStatus status_;
};

}  // namespace autoware::command_mode_switcher

#endif  // EMERGENCY_STOP_HPP_
