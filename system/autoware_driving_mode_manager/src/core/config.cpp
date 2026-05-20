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

#include "config.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace autoware::driving_mode_manager
{

void DrivingModeConfig::define_autoware_mode(const AutowareMode & mode)
{
  autoware_modes_.insert(mode);
}

void DrivingModeConfig::define_trajectory_source(const TrajectorySource & source)
{
  trajectory_sources_.insert(source);
}

void DrivingModeConfig::define_command_source(const CommandSource & source)
{
  command_sources_.insert(source);
}

void DrivingModeConfig::bind_gates(const AutowareMode & mode, const Gates & gates)
{
  if (autoware_modes_.count(mode) == 0) {
    const auto id = std::to_string(mode.id);
    throw std::invalid_argument("unknown autoware mode: " + id);
  }
  if (gates.trajectory && trajectory_sources_.count(*gates.trajectory) == 0) {
    const auto id = std::to_string(gates.trajectory->id);
    throw std::invalid_argument("unknown trajectory source: " + id);
  }
  if (gates.command && command_sources_.count(*gates.command) == 0) {
    const auto id = std::to_string(gates.command->id);
    throw std::invalid_argument("unknown command source: " + id);
  }
  gates_mapping_[mode] = gates;
}

void DrivingModeConfig::bind_operation_mode(
  const AutowareMode & mode, const OperationMode & operation)
{
  if (autoware_modes_.count(mode) == 0) {
    const auto id = std::to_string(mode.id);
    throw std::invalid_argument("unknown autoware mode: " + id);
  }
  operation_to_autoware_[operation] = mode;
  autoware_to_operation_[mode] = operation;
}

void DrivingModeConfig::bind_mrm_behavior(const AutowareMode & mode, const uint16_t behavior)
{
  if (autoware_modes_.count(mode) == 0) {
    const auto id = std::to_string(mode.id);
    throw std::invalid_argument("unknown autoware mode: " + id);
  }
  mrm_behaviors_[mode] = behavior;
}

std::vector<AutowareMode> DrivingModeConfig::autoware_modes() const
{
  return {autoware_modes_.begin(), autoware_modes_.end()};
}

bool DrivingModeConfig::exists(const AutowareMode & mode) const
{
  return autoware_modes_.count(mode) != 0;
}

DrivingModeConfig::Gates DrivingModeConfig::gates(const AutowareMode & mode) const
{
  return gates_mapping_.at(mode);
}

AutowareMode DrivingModeConfig::to_autoware_mode(const OperationMode & operation_mode) const
{
  return operation_to_autoware_.at(operation_mode);
}

OperationMode DrivingModeConfig::to_operation_mode(const AutowareMode & autoware_mode) const
{
  return autoware_to_operation_.at(autoware_mode);
}

uint16_t DrivingModeConfig::to_mrm_behavior(const AutowareMode & mode) const
{
  return mrm_behaviors_.at(mode);
}

}  // namespace autoware::driving_mode_manager
