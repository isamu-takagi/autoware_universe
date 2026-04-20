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

#include "default.hpp"

#include <string>
#include <vector>

// DEBUG
#include <rclcpp/logging.hpp>

namespace autoware::system_mode_decider
{

const auto logger = rclcpp::get_logger("DefaultPlugin");
constexpr auto StopMode = AutowareMode{1001};
constexpr auto AutonomousMode = AutowareMode{1002};
constexpr auto LocalMode = AutowareMode{1003};
constexpr auto RemoteMode = AutowareMode{1004};
constexpr auto EmergencyStop = AutowareMode{2001};
constexpr auto ComfortableStop = AutowareMode{2002};
constexpr auto UnknownMode = AutowareMode{0};

void print_modes(const std::string & title, const std::vector<AutowareMode> & modes)
{
  std::string text;
  for (const auto & mode : modes) {
    text = text + " " + std::to_string(mode.id);
  }
  RCLCPP_INFO_STREAM(logger, title << ":" << text);
}

AutowareMode DefaultPlugin::decide(const CurrentModes & modes, const SystemModeStatus & status)
{
  std::vector<AutowareMode> candidates;
  candidates.push_back(from_operation_mode(modes.operation_mode));
  candidates.push_back(EmergencyStop);
  candidates.push_back(ComfortableStop);
  print_modes("Candidates", candidates);

  std::vector<AutowareMode> availables;
  for (const auto & mode : candidates) {
    if (mode.id != modes.autoware_mode.id) {
      if (status.is_available(mode)) availables.push_back(mode);
    } else {
      if (status.is_continuable(mode)) availables.push_back(mode);
    }
  }
  print_modes("Availables", availables);
  return availables.empty() ? EmergencyStop : availables.front();
};

ModeMapping DefaultPlugin::mapping() const
{
  ModeMapping mapping;

  mapping[StopMode.id] = {};
  mapping[StopMode.id].emplace_back(GateStatus{GateType::kCommandGate, 11});

  mapping[AutonomousMode.id] = {};
  mapping[AutonomousMode.id].emplace_back(GateStatus{GateType::kTrajectoryGate, 100});
  mapping[AutonomousMode.id].emplace_back(GateStatus{GateType::kCommandGate, 12});

  mapping[LocalMode.id] = {};

  mapping[RemoteMode.id] = {};

  mapping[EmergencyStop.id] = {};

  mapping[ComfortableStop.id] = {};

  return mapping;
}

AutowareMode DefaultPlugin::from_operation_mode(const OperationMode & operation_mode) const
{
  // clang-format off
  switch (operation_mode) {
    case OperationMode::kStop:       return StopMode;
    case OperationMode::kAutonomous: return AutonomousMode;
    case OperationMode::kLocal:      return LocalMode;
    case OperationMode::kRemote:     return RemoteMode;
    default:                         return UnknownMode;
  }
  // clang-format on
}

}  // namespace autoware::system_mode_decider

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  autoware::system_mode_decider::DefaultPlugin, autoware::system_mode_decider::Plugin)
