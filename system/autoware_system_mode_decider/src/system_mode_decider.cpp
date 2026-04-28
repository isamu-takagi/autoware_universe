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

  sub_system_mode_status_ = create_subscription<SystemModeStatus>(
    "~/system/driving_mode/status", rclcpp::QoS(1),
    std::bind(&SystemModeDecider::on_system_mode_status, this, _1));
  sub_trajectory_source_ = create_subscription<TrajectorySourceMsg>(
    "~/trajectory/source/status", rclcpp::QoS(1).transient_local(),
    std::bind(&SystemModeDecider::on_trajectory_source, this, _1));
  sub_command_source_ = create_subscription<CommandSourceMsg>(
    "~/command/source/status", rclcpp::QoS(1).transient_local(),
    std::bind(&SystemModeDecider::on_command_source, this, _1));
  sub_control_mode_report_ = create_subscription<ControlModeReport>(
    "~/vehicle/control_mode/report", rclcpp::QoS(1).durability_volatile(),
    std::bind(&SystemModeDecider::on_control_mode_report, this, _1));

  srv_operation_mode_ = create_service<ChangeOperationMode>(
    "~/system/change_operation_mode",
    std::bind(&SystemModeDecider::on_change_operation_mode, this, _1, _2));
  srv_autoware_control_ = create_service<ChangeAutowareControl>(
    "~/system/change_autoware_control",
    std::bind(&SystemModeDecider::on_change_autoware_control, this, _1, _2));

  const auto period = rclcpp::Rate(1.0).period();
  timer_ = rclcpp::create_timer(this, get_clock(), period, [this]() { on_timer_init(); });
}

void SystemModeDecider::on_timer_init()
{
  if (init_flag_ != 0x07) return;
  if (!decider_->access_status().is_ready()) return;

  RCLCPP_INFO_STREAM(get_logger(), "SystemModeDecider is ready.");

  // const auto period = rclcpp::Rate(10.0).period();
  const auto period = rclcpp::Rate(2.0).period();
  timer_->cancel();
  timer_ = rclcpp::create_timer(this, get_clock(), period, [this]() { on_timer_main(); });
}

void SystemModeDecider::on_timer_main()
{
  decider_->update();
}

void SystemModeDecider::on_system_mode_status(const SystemModeStatus & msg)
{
  using SystemModeStatusItem = autoware_system_mode_msgs::msg::SystemModeStatusItem;
  for (const auto & item : msg.items) {
    SystemModeStatusData * data = decider_->access_status().data(AutowareMode{item.mode});
    if (!data) {
      RCLCPP_WARN_STREAM(get_logger(), "unknown status mode: " << item.mode);
      continue;
    }
    switch (item.type) {
      case SystemModeStatusItem::AVAILABLE:
        data->available.update(msg.stamp, item.status);
        break;
      case SystemModeStatusItem::STABLE:
        data->stable.update(msg.stamp, item.status);
        break;
      case SystemModeStatusItem::CONTINUABLE:
        data->continuable.update(msg.stamp, item.status);
        break;
      default:
        RCLCPP_WARN_STREAM(get_logger(), "unknown status type: " << item.type);
        break;
    }
  }
}

void SystemModeDecider::on_trajectory_source(const TrajectorySourceMsg & msg)
{
  init_flag_ |= 0x01;
  decider_->notify_trajectory_source(TrajectorySource{msg.source});
}

void SystemModeDecider::on_command_source(const CommandSourceMsg & msg)
{
  init_flag_ |= 0x02;
  decider_->notify_command_source(CommandSource{msg.source});
}

void SystemModeDecider::on_control_mode_report(const ControlModeReport & msg)
{
  // clang-format off
  const auto convert = [](const ControlModeReport & msg) {
    switch (msg.mode) {
      case ControlModeReport::AUTONOMOUS:               return PlatformMode::kAutoware;
      case ControlModeReport::AUTONOMOUS_STEER_ONLY:    return PlatformMode::kAutowareSteering;
      case ControlModeReport::AUTONOMOUS_VELOCITY_ONLY: return PlatformMode::kAutowareVelocity;
      case ControlModeReport::MANUAL:                   return PlatformMode::kManual;
      default:                                          return PlatformMode::kUnknown;
    }
  };
  // clang-format on
  init_flag_ |= 0x04;
  decider_->notify_vehicle_control_mode(convert(msg));
}

void SystemModeDecider::on_change_operation_mode(
  ChangeOperationMode::Request::SharedPtr req, ChangeOperationMode::Response::SharedPtr res)
{
  // clang-format off
  const auto convert = [](const ChangeOperationMode::Request & req) {
    switch (req.mode) {
      case ChangeOperationMode::Request::STOP:       return OperationMode::kStop;
      case ChangeOperationMode::Request::AUTONOMOUS: return OperationMode::kAutonomous;
      case ChangeOperationMode::Request::LOCAL:      return OperationMode::kLocal;
      case ChangeOperationMode::Request::REMOTE:     return OperationMode::kRemote;
      default:                                       return OperationMode::kUnknown;
    }
  };
  // clang-format on

  // TODO(isamu-takagi): error handling
  decider_->change_operation_mode(convert(*req));
  res->status.success = true;
}

void SystemModeDecider::on_change_autoware_control(
  ChangeAutowareControl::Request::SharedPtr req, ChangeAutowareControl::Response::SharedPtr res)
{
  const auto convert = [](const ChangeAutowareControl::Request & req) {
    return req.autoware_control ? AutowareControl::kEnable : AutowareControl::kDisable;
  };

  // TODO(isamu-takagi): error handling
  decider_->change_autoware_control(convert(*req));
  res->status.success = true;
}

RosInterface::RosInterface(rclcpp::Node * node) : node_(node)
{
  pub_trajectory_select_ =
    node->create_publisher<TrajectorySourceMsg>("~/trajectory/source/select", rclcpp::QoS(1));
  cli_command_select_ = node->create_client<SelectCommandSourceSrv>("~/command/source/select");
  cli_control_mode_command_ =
    node->create_client<ControlModeCommandSrv>("~/vehicle/control_mode/command");
}

rclcpp::Time RosInterface::now() const
{
  return node_->now();
}

void RosInterface::change_trajectory_source(const TrajectorySource & source)
{
  TrajectorySourceMsg msg;
  msg.source = source.id;
  pub_trajectory_select_->publish(msg);
}

void RosInterface::change_command_source(const CommandSource & source)
{
  const auto request = std::make_shared<SelectCommandSourceSrv::Request>();
  request->source = source.id;
  cli_command_select_->async_send_request(request);
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

}  // namespace autoware::system_mode_decider

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::system_mode_decider::SystemModeDecider)
