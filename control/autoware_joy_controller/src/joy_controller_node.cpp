// Copyright 2020 Tier IV, Inc.
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

#include "autoware/joy_controller/joy_controller.hpp"
#include "autoware/joy_controller/joy_converter/ds4_joy_converter.hpp"
#include "autoware/joy_controller/joy_converter/g29_joy_converter.hpp"
#include "autoware/joy_controller/joy_converter/p65_joy_converter.hpp"
#include "autoware/joy_controller/joy_converter/xbox_joy_converter.hpp"

#include <autoware/qos_utils/qos_compatibility.hpp>

#include <algorithm>
#include <memory>
#include <string>

namespace
{
using Gear = autoware_adapi_v1_msgs::msg::Gear;
using TurnIndicators = autoware_adapi_v1_msgs::msg::TurnIndicators;
using HazardLights = autoware_adapi_v1_msgs::msg::HazardLights;

Gear::_status_type getUpperShift(const Gear::_status_type shift)
{
  if (shift == Gear::UNKNOWN) {
    return Gear::PARK;
  }
  if (shift == Gear::PARK) {
    return Gear::REVERSE;
  }
  if (shift == Gear::REVERSE) {
    return Gear::NEUTRAL;
  }
  if (shift == Gear::NEUTRAL) {
    return Gear::DRIVE;
  }
  if (shift == Gear::DRIVE) {
    return Gear::LOW;
  }
  if (shift == Gear::LOW) {
    return Gear::LOW;
  }

  return Gear::UNKNOWN;
}

Gear::_status_type getLowerShift(const Gear::_status_type shift)
{
  if (shift == Gear::UNKNOWN) {
    return Gear::PARK;
  }
  if (shift == Gear::PARK) {
    return Gear::PARK;
  }
  if (shift == Gear::REVERSE) {
    return Gear::PARK;
  }
  if (shift == Gear::NEUTRAL) {
    return Gear::REVERSE;
  }
  if (shift == Gear::DRIVE) {
    return Gear::NEUTRAL;
  }
  if (shift == Gear::LOW) {
    return Gear::DRIVE;
  }

  return Gear::UNKNOWN;
}

const char * getShiftName(const Gear::_status_type shift)
{
  if (shift == Gear::UNKNOWN) {
    return "UNKNOWN";
  }
  if (shift == Gear::PARK) {
    return "PARK";
  }
  if (shift == Gear::REVERSE) {
    return "REVERSE";
  }
  if (shift == Gear::NEUTRAL) {
    return "NEUTRAL";
  }
  if (shift == Gear::DRIVE) {
    return "DRIVE";
  }
  if (shift == Gear::LOW) {
    return "LOW";
  }

  return "NOT_SUPPORTED";
}

const char * getTurnIndicatorsName(const TurnIndicators::_status_type turn_indicators)
{
  if (turn_indicators == TurnIndicators::DISABLE) {
    return "DISABLE";
  }
  if (turn_indicators == TurnIndicators::LEFT) {
    return "LEFT";
  }
  if (turn_indicators == TurnIndicators::RIGHT) {
    return "RIGHT";
  }

  return "NOT_SUPPORTED";
}

const char * getHazardLightsName(const HazardLights::_status_type hazard_lights)
{
  if (hazard_lights == HazardLights::DISABLE) {
    return "DISABLE";
  }
  if (hazard_lights == HazardLights::ENABLE) {
    return "ENABLE";
  }

  return "NOT_SUPPORTED";
}

double calcMapping(const double input, const double sensitivity)
{
  const double exponent = 1.0 / (std::max(0.001, std::min(1.0, sensitivity)));
  return std::pow(input, exponent);
}

}  // namespace

namespace autoware::joy_controller
{
void AutowareJoyControllerNode::onJoy()
{
  const auto msg = sub_joy_.take_data();
  if (!msg) {
    return;
  }

  last_joy_received_time_ = msg->header.stamp;
  if (joy_type_ == "G29") {
    joy_ = std::make_shared<const G29JoyConverter>(*msg);
  } else if (joy_type_ == "DS4") {
    joy_ = std::make_shared<const DS4JoyConverter>(*msg);
  } else if (joy_type_ == "XBOX") {
    joy_ = std::make_shared<const XBOXJoyConverter>(*msg);
  } else {
    joy_ = std::make_shared<const P65JoyConverter>(*msg);
  }

  if (joy_->shift_up() || joy_->shift_down() || joy_->shift_drive() || joy_->shift_reverse()) {
    publishShift();
  }

  if (joy_->turn_signal_left() || joy_->turn_signal_right() || joy_->clear_turn_signal()) {
    publishTurnSignal();
  }

  if (joy_->autoware_engage() || joy_->autoware_disengage()) {
    publishAutowareEngage();
  }

  if (joy_->gate_mode()) {
    switchAutonomousMode();
  }

  if (joy_->vehicle_engage() || joy_->vehicle_disengage()) {
    publishVehicleEngage();
  }

  if (joy_->emergency_stop()) {
    sendEmergencyRequest(true);
  }

  if (joy_->clear_emergency_stop()) {
    sendEmergencyRequest(false);
  }
}

void AutowareJoyControllerNode::onOdometry()
{
  if (raw_control_) {
    return;
  }

  const auto msg = sub_odom_.take_data();
  if (!msg) {
    return;
  }

  auto twist = std::make_shared<geometry_msgs::msg::TwistStamped>();
  twist->header = msg->header;
  twist->twist = msg->twist.twist;

  twist_ = twist;
}

bool AutowareJoyControllerNode::isDataReady()
{
  // Joy
  {
    if (!joy_) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), std::chrono::milliseconds(5000).count(),
        "waiting for joy msg...");
      return false;
    }

    constexpr auto timeout = 2.0;
    const auto time_diff = this->now() - last_joy_received_time_;
    if (time_diff.seconds() > timeout) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), std::chrono::milliseconds(5000).count(), "joy msg is timeout");
      return false;
    }
  }

  // Twist
  if (!raw_control_) {
    if (!twist_) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), std::chrono::milliseconds(5000).count(),
        "waiting for twist msg...");
      return false;
    }

    constexpr auto timeout = 0.5;
    const auto time_diff = this->now() - twist_->header.stamp;
    if (time_diff.seconds() > timeout) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), std::chrono::milliseconds(5000).count(),
        "twist msg is timeout");
      return false;
    }
  }

  return true;
}

void AutowareJoyControllerNode::onTimer()
{
  onOdometry();
  onJoy();

  if (!isDataReady()) {
    return;
  }

  publishControlCommand();
  publishExternalControlCommand();
  publishHeartbeat();
}

void AutowareJoyControllerNode::publishControlCommand()
{
  autoware_control_msgs::msg::Control cmd;
  cmd.stamp = this->now();
  {
    cmd.lateral.steering_tire_angle = steer_ratio_ * joy_->steer();
    cmd.lateral.steering_tire_rotation_rate = steering_angle_velocity_;

    if (joy_->accel()) {
      cmd.longitudinal.acceleration = accel_ratio_ * joy_->accel();
      cmd.longitudinal.velocity =
        twist_->twist.linear.x + velocity_gain_ * cmd.longitudinal.acceleration;
      cmd.longitudinal.velocity =
        std::min(cmd.longitudinal.velocity, static_cast<float>(max_forward_velocity_));
    }

    if (joy_->brake()) {
      cmd.longitudinal.velocity = 0.0;
      cmd.longitudinal.acceleration = -brake_ratio_ * joy_->brake();
    }

    // Backward
    if (joy_->accel() && joy_->brake()) {
      cmd.longitudinal.acceleration = backward_accel_ratio_ * joy_->accel();
      cmd.longitudinal.velocity =
        twist_->twist.linear.x - velocity_gain_ * cmd.longitudinal.acceleration;
      cmd.longitudinal.velocity =
        std::max(cmd.longitudinal.velocity, static_cast<float>(-max_backward_velocity_));
    }
  }

  pub_control_command_->publish(cmd);
  prev_control_command_ = cmd;
}

void AutowareJoyControllerNode::publishExternalControlCommand()
{
  tier4_external_api_msgs::msg::ControlCommandStamped cmd_stamped;
  cmd_stamped.stamp = this->now();
  {
    auto & cmd = cmd_stamped.control;

    cmd.steering_angle = steer_ratio_ * joy_->steer();
    cmd.steering_angle_velocity = steering_angle_velocity_;
    cmd.throttle =
      accel_ratio_ * calcMapping(static_cast<double>(joy_->accel()), accel_sensitivity_);
    cmd.brake = brake_ratio_ * calcMapping(static_cast<double>(joy_->brake()), brake_sensitivity_);
  }

  pub_external_control_command_->publish(cmd_stamped);
  prev_external_control_command_ = cmd_stamped.control;
}

void AutowareJoyControllerNode::publishShift()
{
  GearCommand gear;
  gear.stamp = this->now();
  gear.command.status = prev_gear_;

  if (joy_->shift_up()) {
    gear.command.status = getUpperShift(prev_gear_);
  }

  if (joy_->shift_down()) {
    gear.command.status = getLowerShift(prev_gear_);
  }

  if (joy_->shift_drive()) {
    gear.command.status = Gear::DRIVE;
  }

  if (joy_->shift_reverse()) {
    gear.command.status = Gear::REVERSE;
  }

  RCLCPP_INFO(get_logger(), "Gear::%s", getShiftName(gear.command.status));

  pub_gear_->publish(gear);
  prev_gear_ = gear.command.status;
}

void AutowareJoyControllerNode::publishTurnSignal()
{
  TurnIndicatorsCommand turn_indicators;
  turn_indicators.stamp = this->now();
  HazardLightsCommand hazard_lights;
  hazard_lights.stamp = turn_indicators.stamp;

  if (joy_->turn_signal_left() && joy_->turn_signal_right()) {
    turn_indicators.command.status = TurnIndicators::DISABLE;
    hazard_lights.command.status = HazardLights::ENABLE;
  } else if (joy_->turn_signal_left()) {
    turn_indicators.command.status = TurnIndicators::LEFT;
    hazard_lights.command.status = HazardLights::DISABLE;
  } else if (joy_->turn_signal_right()) {
    turn_indicators.command.status = TurnIndicators::RIGHT;
    hazard_lights.command.status = HazardLights::DISABLE;
  }

  if (joy_->clear_turn_signal()) {
    turn_indicators.command.status = TurnIndicators::DISABLE;
    hazard_lights.command.status = HazardLights::DISABLE;
  }

  RCLCPP_INFO(
    get_logger(), "TurnIndicators::%s, HazardLights::%s",
    getTurnIndicatorsName(turn_indicators.command.status),
    getHazardLightsName(hazard_lights.command.status));

  pub_turn_indicators_->publish(turn_indicators);
  pub_hazard_lights_->publish(hazard_lights);
}

void AutowareJoyControllerNode::publishHeartbeat()
{
  ManualOperatorHeartbeat heartbeat;
  heartbeat.stamp = this->now();
  heartbeat.ready = true;
  pub_heartbeat_->publish(heartbeat);
}

void AutowareJoyControllerNode::sendEmergencyRequest(bool emergency)
{
  RCLCPP_INFO(get_logger(), "%s emergency stop", emergency ? "Set" : "Clear");

  auto request = std::make_shared<SendMrmRequest::Request>();
  request->request.sender = "autoware_joy_controller";
  request->request.strategy = emergency ? MrmRequest::DELEGATE : MrmRequest::CANCEL;

  client_mrm_request_->async_send_request(
    request, [this](rclcpp::Client<SendMrmRequest>::SharedFuture result) {
      auto response = result.get();
      if (response->status.success) {
        RCLCPP_INFO(get_logger(), "service succeeded");
      } else {
        RCLCPP_WARN(get_logger(), "service failed: %s", response->status.message.c_str());
      }
    });
}

void AutowareJoyControllerNode::publishAutowareEngage()
{
  if (joy_->autoware_engage()) {
    RCLCPP_INFO(get_logger(), "Autoware Engage");
    sendChangeOperationMode(client_change_to_local_);
  }

  if (joy_->autoware_disengage()) {
    RCLCPP_INFO(get_logger(), "Autoware Disengage");
    sendChangeOperationMode(client_change_to_stop_);
  }
}

void AutowareJoyControllerNode::switchAutonomousMode()
{
  const auto state = sub_operation_mode_.take_data();
  if (!state) {
    RCLCPP_WARN(get_logger(), "operation mode state is not received");
    return;
  }

  if (state->mode == OperationModeState::AUTONOMOUS) {
    RCLCPP_INFO(get_logger(), "Switch to Local Mode");
    sendChangeOperationMode(client_change_to_local_);
  } else if (state->mode == OperationModeState::LOCAL) {
    RCLCPP_INFO(get_logger(), "Switch to Autonomous Mode");
    sendChangeOperationMode(client_change_to_autonomous_);
  }
}

void AutowareJoyControllerNode::publishVehicleEngage()
{
  if (joy_->vehicle_engage()) {
    RCLCPP_INFO(get_logger(), "Vehicle Engage");
    sendChangeOperationMode(client_enable_autoware_control_);
  }

  if (joy_->vehicle_disengage()) {
    RCLCPP_INFO(get_logger(), "Vehicle Disengage");
    sendChangeOperationMode(client_disable_autoware_control_);
  }
}

void AutowareJoyControllerNode::sendChangeOperationMode(
  const rclcpp::Client<ChangeOperationMode>::SharedPtr & client)
{
  if (!client->service_is_ready()) {
    RCLCPP_WARN(get_logger(), "%s is unavailable", client->get_service_name());
    return;
  }

  const auto req = std::make_shared<ChangeOperationMode::Request>();
  client->async_send_request(
    req, [this, client](rclcpp::Client<ChangeOperationMode>::SharedFuture result) {
      const auto & status = result.get()->status;
      RCLCPP_INFO(
        get_logger(), "%s: %s, %d, %s", client->get_service_name(),
        status.success ? "success" : "failure", status.code, status.message.c_str());
    });
}

void AutowareJoyControllerNode::initTimer(double period_s)
{
  const auto period_ns =
    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(period_s));
  timer_ = rclcpp::create_timer(
    this, get_clock(), period_ns, std::bind(&AutowareJoyControllerNode::onTimer, this));
}

AutowareJoyControllerNode::AutowareJoyControllerNode(const rclcpp::NodeOptions & node_options)
: Node("autoware_joy_controller", node_options)
{
  // Parameter
  joy_type_ = declare_parameter<std::string>("joy_type");
  update_rate_ = declare_parameter<double>("update_rate");
  accel_ratio_ = declare_parameter<double>("accel_ratio");
  brake_ratio_ = declare_parameter<double>("brake_ratio");
  steer_ratio_ = declare_parameter<double>("steer_ratio");
  steering_angle_velocity_ = declare_parameter<double>("steering_angle_velocity");
  accel_sensitivity_ = declare_parameter<double>("accel_sensitivity");
  brake_sensitivity_ = declare_parameter<double>("brake_sensitivity");
  raw_control_ = declare_parameter<bool>("control_command.raw_control");
  velocity_gain_ = declare_parameter<double>("control_command.velocity_gain");
  max_forward_velocity_ = declare_parameter<double>("control_command.max_forward_velocity");
  max_backward_velocity_ = declare_parameter<double>("control_command.max_backward_velocity");
  backward_accel_ratio_ = declare_parameter<double>("control_command.backward_accel_ratio");

  RCLCPP_INFO(get_logger(), "Joy type: %s", joy_type_.c_str());

  // Callback Groups
  callback_group_services_ =
    this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  // Subscriber
  if (raw_control_) {
    twist_ = std::make_shared<geometry_msgs::msg::TwistStamped>();
  }

  // Publisher
  pub_control_command_ =
    this->create_publisher<autoware_control_msgs::msg::Control>("output/control_command", 1);
  pub_external_control_command_ =
    this->create_publisher<tier4_external_api_msgs::msg::ControlCommandStamped>(
      "output/external_control_command", 1);
  pub_gear_ = create_publisher<GearCommand>("output/gear", 1);
  pub_turn_indicators_ = create_publisher<TurnIndicatorsCommand>("output/turn_indicators", 1);
  pub_hazard_lights_ = create_publisher<HazardLightsCommand>("output/hazard_lights", 1);
  pub_heartbeat_ = create_publisher<ManualOperatorHeartbeat>("output/heartbeat", 1);

  // Service Client
  client_mrm_request_ = create_client<SendMrmRequest>(
    "service/mrm_request", AUTOWARE_DEFAULT_SERVICES_QOS_PROFILE(), callback_group_services_);
  while (!client_mrm_request_->wait_for_service(std::chrono::seconds(1))) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(get_logger(), "Interrupted while waiting for service.");
      rclcpp::shutdown();
      return;
    }
    RCLCPP_INFO(get_logger(), "Waiting for mrm_request service connection...");
  }

  client_change_to_local_ = create_client<ChangeOperationMode>("service/change_to_local");
  client_change_to_stop_ = create_client<ChangeOperationMode>("service/change_to_stop");
  client_change_to_autonomous_ = create_client<ChangeOperationMode>("service/change_to_autonomous");
  client_enable_autoware_control_ =
    create_client<ChangeOperationMode>("service/enable_autoware_control");
  client_disable_autoware_control_ =
    create_client<ChangeOperationMode>("service/disable_autoware_control");

  // Timer
  initTimer(1.0 / update_rate_);
}
}  // namespace autoware::joy_controller

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::joy_controller::AutowareJoyControllerNode)
