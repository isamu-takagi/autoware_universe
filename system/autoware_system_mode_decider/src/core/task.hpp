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
#include <string>

namespace autoware::system_mode_decider
{

struct GateStatusItem
{
  TrajectorySource trajectory_source;
  CommandSource command_source;
  PlatformMode platform_mode;
};

struct GateStatus
{
  GateStatusItem status;
  GateStatusItem expect;
};

enum class TaskResult {
  kFinished,
  kRunning,
  kTimeout,
};

class Task
{
public:
  virtual ~Task() = default;
  virtual TaskResult execute(Interface & interface, GateStatus & gates) = 0;
  virtual std::string describe() const = 0;
};

class NoneTask : public Task
{
public:
  TaskResult execute(Interface & interface, GateStatus & gates) override;
  std::string describe() const override;
};

class TrajectorySourceTask : public Task
{
public:
  explicit TrajectorySourceTask(const TrajectorySource & target) : target_(target) {}
  TaskResult execute(Interface & interface, GateStatus & gates) override;
  std::string describe() const override;

private:
  static constexpr double timeout = 3.0;
  TrajectorySource target_;
  std::optional<rclcpp::Time> stamp_;
};

class CommandSourceTask : public Task
{
public:
  explicit CommandSourceTask(const CommandSource & target) : target_(target) {}
  TaskResult execute(Interface & interface, GateStatus & gates) override;
  std::string describe() const override;

private:
  static constexpr double timeout = 3.0;
  CommandSource target_;
  std::optional<rclcpp::Time> stamp_;
};

class PlatformModeTask : public Task
{
public:
  explicit PlatformModeTask(const PlatformMode & target) : target_(target) {}
  TaskResult execute(Interface & interface, GateStatus & gates) override;
  std::string describe() const override;

private:
  PlatformMode target_;
  std::optional<rclcpp::Time> stamp_;
};

}  // namespace autoware::system_mode_decider

#endif  // CORE__TASK_HPP_
