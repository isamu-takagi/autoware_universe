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

#include "ros_interface.hpp"

#include <memory>

namespace autoware::driving_mode_manager
{

RosInterface::RosInterface(rclcpp::Node * node) : node_(node)
{
  cli_trajectory_source_ = node->create_client<TrajectorySourceSrv>("~/trajectory/source/change");
  cli_command_source_ = node->create_client<ChangeCommandSourceSrv>("~/command/source/change");
  cli_control_mode_command_ =
    node->create_client<ControlModeCommandSrv>("~/vehicle/control_mode/command");
  pub_operation_mode_ = node->create_publisher<OperationModeStateMsg>(
    "~/system/operation_mode_state", rclcpp::QoS(1).transient_local());
}

rclcpp::Time RosInterface::now() const
{
  return node_->now();
}

void RosInterface::change_trajectory_source(const TrajectorySource & source)
{
  const auto request = std::make_shared<TrajectorySourceSrv::Request>();
  request->source = source.id;
  cli_trajectory_source_->async_send_request(request);
}

void RosInterface::change_command_source(const CommandSource & source)
{
  const auto request = std::make_shared<ChangeCommandSourceSrv::Request>();
  request->source = source.id;
  cli_command_source_->async_send_request(request);
}

void RosInterface::change_platform_mode(const PlatformMode & mode)
{
  // clang-format off
  const auto convert = [](const PlatformMode & mode) -> std::optional<uint8_t> {
    using Command = ControlModeCommandSrv::Request;
    switch (mode) {
      case PlatformMode::kAutoware:         return Command::AUTONOMOUS;
      case PlatformMode::kAutowareSteering: return Command::AUTONOMOUS_STEER_ONLY;
      case PlatformMode::kAutowareVelocity: return Command::AUTONOMOUS_VELOCITY_ONLY;
      case PlatformMode::kManual:           return Command::MANUAL;
      default:                              return std::nullopt;
    }
  };
  // clang-format on

  const auto command = convert(mode);
  if (!command) {
    RCLCPP_ERROR_STREAM(node_->get_logger(), "unknown platform mode");
    return;
  }
  const auto request = std::make_shared<ControlModeCommandSrv::Request>();
  request->mode = command.value();
  cli_control_mode_command_->async_send_request(request);
}

void RosInterface::publish_operation_mode(const OperationModeState & state) const
{
  const auto convert = [](const OperationMode & mode) {
    // clang-format off
    switch (mode) {
      case OperationMode::kStop:       return OperationModeStateMsg::STOP;
      case OperationMode::kAutonomous: return OperationModeStateMsg::AUTONOMOUS;
      case OperationMode::kLocal:      return OperationModeStateMsg::LOCAL;
      case OperationMode::kRemote:     return OperationModeStateMsg::REMOTE;
      default:                         return OperationModeStateMsg::UNKNOWN;
    }
    // clang-format on
  };

  OperationModeStateMsg msg;
  msg.stamp = now();
  msg.mode = convert(state.mode);
  msg.is_autoware_control_enabled = state.is_autoware_control_enabled;
  msg.is_in_transition = state.is_in_transition;
  msg.is_stop_mode_available = state.is_stop_mode_available;
  msg.is_autonomous_mode_available = state.is_autonomous_mode_available;
  msg.is_local_mode_available = state.is_local_mode_available;
  msg.is_remote_mode_available = state.is_remote_mode_available;
  pub_operation_mode_->publish(msg);
}

}  // namespace autoware::driving_mode_manager

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::driving_mode_manager::DrivingModeManager)
