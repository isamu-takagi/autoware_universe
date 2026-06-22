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

#include <string>

void MockInterface::init(MainLogic * logic)
{
  (void)logic;
}

rclcpp::Time MockInterface::now() const
{
  return rclcpp::Time{};
}

void MockInterface::change_trajectory_source(const TrajectorySource & source)
{
  (void)source;
}

void MockInterface::change_command_source(const CommandSource & source)
{
  (void)source;
}

void MockInterface::change_command_filter(const CommandFilter & filter)
{
  (void)filter;
}

void MockInterface::change_platform_mode(const PlatformMode & mode)
{
  (void)mode;
}

void MockInterface::publish_operation_mode(const OperationModeState & state) const
{
  (void)state;
}

void MockInterface::publish_mrm_state(const MrmState & state) const
{
  (void)state;
}

void MockInterface::publish_driving_mode_request(const AutowareMode & mode) const
{
  (void)mode;
}

void MockInterface::publish_driving_mode_info(const ModeInfo & info) const
{
  (void)info;
}

void MockInterface::publish_debug(const DebugStatus & status) const
{
  (void)status;
}

void MockInterface::publish_debug(const RequestModes & request) const
{
  (void)request;
}

void MockInterface::log_info(const std::string & message) const
{
  (void)message;
}

void MockInterface::log_warn(const std::string & message) const
{
  (void)message;
}

void MockInterface::log_error(const std::string & message) const
{
  (void)message;
}

void MockInterface::log_debug(const std::string & message) const
{
  (void)message;
}
