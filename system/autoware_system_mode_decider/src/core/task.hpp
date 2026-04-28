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

#ifndef CORE__TASK_HPP_
#define CORE__TASK_HPP_

#include "type/interface.hpp"

#include <rclcpp/time.hpp>

#include <optional>

namespace autoware::system_mode_decider
{

struct GateStatusItem
{
  TrajectorySource trajectory_source;
  CommandSource command_source;
  PlatformMode platform_mode;
};

struct GateStatusTemp
{
  GateStatusItem status;
  GateStatusItem expect;
};

class Task
{
public:
  virtual ~Task() = default;
  virtual void execute(Interface & interface) = 0;
  virtual bool timeout(Interface & interface) = 0;
  virtual bool expects(const GateStatus & status) const = 0;
};

class NoneTask : public Task
{
public:
  void execute(Interface & interface) override;
  bool timeout(Interface & interface) override;
  bool expects(const GateStatus & status) const override;
};

class GateTask : public Task
{
public:
  explicit GateTask(const GateStatus & target) : target_(target) {}
  void execute(Interface & interface) override;
  bool timeout(Interface & interface) override;
  bool expects(const GateStatus & status) const override;

private:
  GateStatus target_;
  std::optional<rclcpp::Time> stamp_;
};

class VehicleControlModeTask : public Task
{
public:
  explicit VehicleControlModeTask(const AutowareControl & target) : target_(target) {}
  void execute(Interface & interface) override;
  bool timeout(Interface & interface) override;
  bool expects(const GateStatus & status) const override;

private:
  AutowareControl target_;
  std::optional<rclcpp::Time> stamp_;
};

}  // namespace autoware::system_mode_decider

#endif  // CORE__TASK_HPP_
