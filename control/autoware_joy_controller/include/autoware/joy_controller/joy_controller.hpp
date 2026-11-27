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

#ifndef AUTOWARE__JOY_CONTROLLER__JOY_CONTROLLER_HPP_
#define AUTOWARE__JOY_CONTROLLER__JOY_CONTROLLER_HPP_

#include "autoware/joy_controller/joy_converter/joy_converter_base.hpp"

#include <autoware_utils/ros/polling_subscriber.hpp>
#include <rclcpp/rclcpp.hpp>

#include <autoware_adapi_v1_msgs/msg/acceleration_command.hpp>
#include <autoware_adapi_v1_msgs/msg/gear_command.hpp>
#include <autoware_adapi_v1_msgs/msg/hazard_lights_command.hpp>
#include <autoware_adapi_v1_msgs/msg/manual_control_mode_status.hpp>
#include <autoware_adapi_v1_msgs/msg/manual_operator_heartbeat.hpp>
#include <autoware_adapi_v1_msgs/msg/operation_mode_state.hpp>
#include <autoware_adapi_v1_msgs/msg/pedals_command.hpp>
#include <autoware_adapi_v1_msgs/msg/steering_command.hpp>
#include <autoware_adapi_v1_msgs/msg/turn_indicators_command.hpp>
#include <autoware_adapi_v1_msgs/msg/velocity_command.hpp>
#include <autoware_adapi_v1_msgs/srv/change_operation_mode.hpp>
#include <autoware_adapi_v1_msgs/srv/select_manual_control_mode.hpp>
#include <autoware_adapi_v1_msgs/srv/send_mrm_request.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/joy.hpp>

#include <memory>
#include <string>

namespace autoware::joy_controller
{
class AutowareJoyControllerNode : public rclcpp::Node
{
  using OperationModeState = autoware_adapi_v1_msgs::msg::OperationModeState;
  using ChangeOperationMode = autoware_adapi_v1_msgs::srv::ChangeOperationMode;
  using ManualControlMode = autoware_adapi_v1_msgs::msg::ManualControlMode;
  using ManualControlModeStatus = autoware_adapi_v1_msgs::msg::ManualControlModeStatus;
  using SelectManualControlMode = autoware_adapi_v1_msgs::srv::SelectManualControlMode;
  using SendMrmRequest = autoware_adapi_v1_msgs::srv::SendMrmRequest;
  using MrmRequest = autoware_adapi_v1_msgs::msg::MrmRequest;
  using PedalsCommand = autoware_adapi_v1_msgs::msg::PedalsCommand;
  using SteeringCommand = autoware_adapi_v1_msgs::msg::SteeringCommand;
  using AccelerationCommand = autoware_adapi_v1_msgs::msg::AccelerationCommand;
  using VelocityCommand = autoware_adapi_v1_msgs::msg::VelocityCommand;
  using Gear = autoware_adapi_v1_msgs::msg::Gear;
  using GearCommand = autoware_adapi_v1_msgs::msg::GearCommand;
  using TurnIndicators = autoware_adapi_v1_msgs::msg::TurnIndicators;
  using TurnIndicatorsCommand = autoware_adapi_v1_msgs::msg::TurnIndicatorsCommand;
  using HazardLights = autoware_adapi_v1_msgs::msg::HazardLights;
  using HazardLightsCommand = autoware_adapi_v1_msgs::msg::HazardLightsCommand;
  using ManualOperatorHeartbeat = autoware_adapi_v1_msgs::msg::ManualOperatorHeartbeat;

public:
  explicit AutowareJoyControllerNode(const rclcpp::NodeOptions & node_options);

private:
  // Parameter
  std::string joy_type_;
  double update_rate_;
  double accel_ratio_;
  double brake_ratio_;
  double steer_ratio_;
  double steering_angle_velocity_;
  double accel_sensitivity_;
  double brake_sensitivity_;

  // ControlCommand Parameter
  bool raw_control_;
  double velocity_gain_;
  double max_forward_velocity_;
  double max_backward_velocity_;
  double backward_accel_ratio_;

  // CallbackGroups
  rclcpp::CallbackGroup::SharedPtr callback_group_services_;

  // Subscriber
  autoware_utils::InterProcessPollingSubscriber<sensor_msgs::msg::Joy> sub_joy_{this, "input/joy"};
  autoware_utils::InterProcessPollingSubscriber<nav_msgs::msg::Odometry> sub_odom_{
    this, "input/odometry"};
  autoware_utils::InterProcessPollingSubscriber<OperationModeState> sub_operation_mode_{
    this, "input/operation_mode_state", rclcpp::QoS{1}.transient_local()};
  autoware_utils::InterProcessPollingSubscriber<ManualControlModeStatus> sub_control_mode_{
    this, "input/control_mode_status", rclcpp::QoS{1}.transient_local()};

  rclcpp::Time last_joy_received_time_;
  std::shared_ptr<const JoyConverterBase> joy_;
  geometry_msgs::msg::TwistStamped::ConstSharedPtr twist_;

  void onJoy();
  void onOdometry();

  // Publisher
  rclcpp::Publisher<PedalsCommand>::SharedPtr pub_pedals_;
  rclcpp::Publisher<SteeringCommand>::SharedPtr pub_steering_;
  rclcpp::Publisher<AccelerationCommand>::SharedPtr pub_acceleration_;
  rclcpp::Publisher<VelocityCommand>::SharedPtr pub_velocity_;
  rclcpp::Publisher<GearCommand>::SharedPtr pub_gear_;
  rclcpp::Publisher<TurnIndicatorsCommand>::SharedPtr pub_turn_indicators_;
  rclcpp::Publisher<HazardLightsCommand>::SharedPtr pub_hazard_lights_;
  rclcpp::Publisher<ManualOperatorHeartbeat>::SharedPtr pub_heartbeat_;

  void publishControlCommand();
  void publishExternalControlCommand();
  void publishShift();
  void publishTurnSignal();
  void publishHeartbeat();
  void publishAutowareEngage();
  void switchAutonomousMode();
  void publishVehicleEngage();
  void sendEmergencyRequest(bool emergency);
  void sendChangeOperationMode(const rclcpp::Client<ChangeOperationMode>::SharedPtr & client);

  // Service Client
  rclcpp::Client<SendMrmRequest>::SharedPtr client_mrm_request_;
  rclcpp::Client<ChangeOperationMode>::SharedPtr client_change_to_local_;
  rclcpp::Client<ChangeOperationMode>::SharedPtr client_change_to_stop_;
  rclcpp::Client<ChangeOperationMode>::SharedPtr client_change_to_autonomous_;
  rclcpp::Client<SelectManualControlMode>::SharedPtr client_select_control_mode_;
  rclcpp::Client<ChangeOperationMode>::SharedPtr client_enable_autoware_control_;
  rclcpp::Client<ChangeOperationMode>::SharedPtr client_disable_autoware_control_;

  // Previous State
  Gear::_status_type prev_gear_ = Gear::UNKNOWN;

  // Timer
  rclcpp::TimerBase::SharedPtr timer_;
  void initTimer(double period_s);

  bool isDataReady();

  // Manual control mode
  bool is_control_mode_ready_ = false;
  bool is_control_mode_requesting_ = false;
  void checkControlMode();
  void onTimer();
};
}  // namespace autoware::joy_controller

#endif  // AUTOWARE__JOY_CONTROLLER__JOY_CONTROLLER_HPP_
