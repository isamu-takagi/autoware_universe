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

#ifndef TYPE__INTERFACE_HPP_
#define TYPE__INTERFACE_HPP_

#include <autoware_driving_mode_manager/types.hpp>
#include <rclcpp/time.hpp>

namespace autoware::driving_mode_manager
{

struct OperationModeState
{
  OperationMode mode;
  bool is_autoware_control_enabled;
  bool is_in_transition;
  bool is_stop_mode_available;
  bool is_autonomous_mode_available;
  bool is_local_mode_available;
  bool is_remote_mode_available;
};

class Interface
{
public:
  virtual ~Interface() = default;
  virtual rclcpp::Time now() const = 0;
  virtual void change_trajectory_source(const TrajectorySource & source) = 0;
  virtual void change_command_source(const CommandSource & source) = 0;
  virtual void change_platform_mode(const PlatformMode & mode) = 0;
};

}  // namespace autoware::driving_mode_manager

#endif  // TYPE__INTERFACE_HPP_
