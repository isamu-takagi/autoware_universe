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

#ifndef AUTOWARE_SYSTEM_MODE_DECIDER__CONFIG_HPP_
#define AUTOWARE_SYSTEM_MODE_DECIDER__CONFIG_HPP_

#include "types.hpp"

#include <optional>
#include <vector>

namespace autoware::system_mode_decider
{

class DrivingModeConfig
{
public:
  struct Gates
  {
    std::optional<TrajectorySource> trajectory_source;
    std::optional<CommandSource> command_source;
  };

  void define_autoware_mode(const AutowareMode & mode);
  void define_trajectory_source(const TrajectorySource & source);
  void define_command_source(const CommandSource & source);
  void bind_gates(const AutowareMode & mode, const Gates & gates);
  void bind_operation_mode(const AutowareMode & mode, const OperationMode & operation_mode);

  std::vector<AutowareMode> autoware_modes() const;
  bool exists(const AutowareMode & mode) const;
  Gates gates(const AutowareMode & mode) const;
  AutowareMode from_operation_mode(const OperationMode & operation_mode) const;
};

}  // namespace autoware::system_mode_decider

#endif  // AUTOWARE_SYSTEM_MODE_DECIDER__CONFIG_HPP_
