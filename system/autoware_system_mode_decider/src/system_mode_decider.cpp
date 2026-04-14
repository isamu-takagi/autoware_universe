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

#include <memory>
#include <string>

namespace autoware::system_mode_decider
{

SystemModeDecider::SystemModeDecider(const rclcpp::NodeOptions & options)
: Node("system_mode_decider", options),
  diag_(this, 0.1),
  loader_("autoware_system_mode_decider", "autoware::system_mode_decider::Plugin")
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  diag_.setHardwareID("none");

  const auto plugin_name = declare_parameter<std::string>("plugin");
  if (!loader_.isClassAvailable(plugin_name)) {
    throw std::invalid_argument("unknown plugin: " + plugin_name);
  }
  const auto plugin = loader_.createSharedInstance(plugin_name);
  decider_ = std::make_unique<Decider>(std::make_unique<RosInterface>(this), plugin);

  sub_trajectory_source_ = create_subscription<TrajectorySource>(
    "~/trajectory/source/status", rclcpp::QoS(1).transient_local(),
    std::bind(&SystemModeDecider::on_trajectory_source, this, _1));
  sub_command_source_ = create_subscription<CommandSource>(
    "~/command/source/status", rclcpp::QoS(1).transient_local(),
    std::bind(&SystemModeDecider::on_command_source, this, _1));
  sub_vehicle_source_ = create_subscription<VehicleSource>(
    "~/vehicle/source/status", rclcpp::QoS(1).durability_volatile(),
    std::bind(&SystemModeDecider::on_vehicle_source, this, _1));

  srv_operation_mode_ = create_service<ChangeOperationMode>(
    "~/system/change_operation_mode",
    std::bind(&SystemModeDecider::on_change_operation_mode, this, _1, _2));

  const auto period = rclcpp::Rate(1.0).period();
  timer_ = rclcpp::create_timer(this, get_clock(), period, [this]() { on_timer_init(); });
}

void SystemModeDecider::on_timer_init()
{
  if (!ready()) return;
  RCLCPP_INFO_STREAM(get_logger(), "SystemModeDecider is ready.");

  const auto period = rclcpp::Rate(10.0).period();
  timer_->cancel();
  timer_ = rclcpp::create_timer(this, get_clock(), period, [this]() { on_timer_main(); });
}

void SystemModeDecider::on_timer_main()
{
  decider_->update();
}

void SystemModeDecider::on_trajectory_source(const TrajectorySource & msg)
{
  init_flag_ |= 0x01;
  decider_->notify_gate_status(GateStatus{GateType::kTrajectoryGate, msg.source});
}

void SystemModeDecider::on_command_source(const CommandSource & msg)
{
  init_flag_ |= 0x02;
  decider_->notify_gate_status(GateStatus{GateType::kCommandGate, msg.source});
}

void SystemModeDecider::on_vehicle_source(const VehicleSource & msg)
{
  init_flag_ |= 0x04;
  decider_->notify_gate_status(GateStatus{GateType::kVehicleDriver, msg.mode});
}

void SystemModeDecider::on_change_operation_mode(
  ChangeOperationMode::Request::SharedPtr req, ChangeOperationMode::Response::SharedPtr res)
{
  decider_->change_autoware_mode(AutowareMode{req->mode});
  res->status.success = true;
}

RosInterface::RosInterface(rclcpp::Node * node) : node_(node)
{
  pub_trajectory_select_ =
    node->create_publisher<TrajectorySource>("~/trajectory/source/select", rclcpp::QoS(1));
  cli_command_select_ = node->create_client<SelectCommandSource>("~/command/source/select");
}

rclcpp::Time RosInterface::now() const
{
  return node_->now();
}

void RosInterface::change_gate_status(const GateStatus & status)
{
  const auto type_name = [](GateType type) {
    switch (type) {
      case GateType::kInvalid:
        return "Invalid";
      case GateType::kTrajectoryGate:
        return "TrajectoryGate";
      case GateType::kCommandGate:
        return "CommandGate";
      case GateType::kCommandFilter:
        return "CommandFilter";
      case GateType::kVehicleDriver:
        return "VehicleDriver";
      default:
        return "Unknown";
    }
  };
  RCLCPP_INFO_STREAM(
    node_->get_logger(), "Change gate status: " << type_name(status.type) << ", " << status.id);

  // clang-format off
  switch (status.type) {
    case GateType::kTrajectoryGate: return change_trajectory_source(status.id);
    case GateType::kCommandGate:    return change_command_source(status.id);
    default:                        return;
  }
  // clang-format on
}

void RosInterface::change_trajectory_source(uint32_t id)
{
  TrajectorySource msg;
  msg.source = id;
  pub_trajectory_select_->publish(msg);
}

void RosInterface::change_command_source(uint32_t id)
{
  const auto request = std::make_shared<SelectCommandSource::Request>();
  request->source = id;
  cli_command_select_->async_send_request(request);
}

}  // namespace autoware::system_mode_decider

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::system_mode_decider::SystemModeDecider)
