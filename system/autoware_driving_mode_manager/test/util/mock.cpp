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

#include "mock.hpp"

#include <iostream>
#include <string>

void MockInterface::init(MainLogic * logic)
{
  logic_ = logic;
}

rclcpp::Time MockInterface::now() const
{
  return rclcpp::Time{};
}

void MockInterface::change_trajectory_source(const TrajectorySource & source)
{
  std::cerr << "[mock] change_trajectory_source:" << source.id << std::endl;
  trajectory_source = source;
  logic_->on_trajectory_source(source);
}

void MockInterface::change_command_source(const CommandSource & source)
{
  std::cerr << "[mock] change_command_source:" << source.id << std::endl;
  command_source = source;
  logic_->on_command_source(source);
}

void MockInterface::change_command_filter(const CommandFilter & filter)
{
  std::cerr << "[mock] change_command_filter:" << filter.flag << std::endl;
  command_filter = filter;
  logic_->on_command_filter(filter);
}

void MockInterface::change_platform_mode(const PlatformMode & mode)
{
  std::cerr << "[mock] change_platform_mode" << std::endl;
  platform_mode = mode;
  logic_->on_vehicle_control_mode(mode);
}

void MockInterface::publish_operation_mode(const OperationModeState & state) const
{
  (void)state;
  std::cerr << "[mock] publish_operation_mode" << std::endl;
}

void MockInterface::publish_mrm_state(const MrmState & state) const
{
  (void)state;
  std::cerr << "[mock] publish_mrm_state" << std::endl;
}

void MockInterface::publish_driving_mode_request(const AutowareMode & mode) const
{
  (void)mode;
  std::cerr << "[mock] publish_driving_mode_request" << std::endl;
}

void MockInterface::publish_driving_mode_info(const ModeInfo & info) const
{
  (void)info;
  std::cerr << "[mock] publish_driving_mode_info" << std::endl;
}

void MockInterface::publish_debug(const DebugStatus & status) const
{
  (void)status;
  std::cerr << "[mock] publish_debug_status" << std::endl;
}

void MockInterface::publish_debug(const RequestModes & request) const
{
  (void)request;
  std::cerr << "[mock] publish_debug_request" << std::endl;
}

void MockInterface::log_info(const std::string & message) const
{
  std::cerr << "[mock] log_info: " << message << std::endl;
}

void MockInterface::log_warn(const std::string & message) const
{
  std::cerr << "[mock] log_warn: " << message << std::endl;
}

void MockInterface::log_error(const std::string & message) const
{
  std::cerr << "[mock] log_error: " << message << std::endl;
}

void MockInterface::log_debug(const std::string & message) const
{
  std::cerr << "[mock] log_debug: " << message << std::endl;
}
