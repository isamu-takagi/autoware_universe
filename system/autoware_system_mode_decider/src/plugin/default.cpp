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

#include <vector>

// DEBUG
#include <rclcpp/logging.hpp>

namespace autoware::system_mode_decider
{

AutowareMode DefaultPlugin::decide(const CurrentModes & modes, const SystemModeStatus & status)
{
  std::vector<AutowareMode> candidates;
  candidates.push_back(from_operation_mode(modes.operation_mode));

  std::vector<AutowareMode> availables;
  for (const auto & mode : candidates) {
    if (mode.id != modes.autoware_mode.id) {
      if (status.is_available(mode)) availables.push_back(mode);
    } else {
      if (status.is_continuable(mode)) availables.push_back(mode);
    }
  }

  RCLCPP_INFO_STREAM(rclcpp::get_logger("DefaultPlugin"), "Available modes:");
  for (const auto & mode : availables) {
    RCLCPP_INFO_STREAM(rclcpp::get_logger("DefaultPlugin"), "  - " << mode.id);
  }
  return AutowareMode{2};
};

ModeMapping DefaultPlugin::mapping() const
{
  ModeMapping mapping;

  // Stop mode
  mapping[1] = {};
  mapping[1].emplace_back(GateStatus{GateType::kCommandGate, 11});

  // Auto mode
  mapping[2] = {};
  mapping[2].emplace_back(GateStatus{GateType::kTrajectoryGate, 100});
  mapping[2].emplace_back(GateStatus{GateType::kCommandGate, 12});

  return mapping;
}

AutowareMode DefaultPlugin::from_operation_mode(const OperationMode & operation_mode) const
{
  // clang-format off
  switch (operation_mode) {
    case OperationMode::kStop:       return AutowareMode{1};
    case OperationMode::kAutonomous: return AutowareMode{2};
    case OperationMode::kLocal:      return AutowareMode{3};
    case OperationMode::kRemote:     return AutowareMode{4};
    default:                         return AutowareMode{0};
  }
  // clang-format on
}

}  // namespace autoware::system_mode_decider

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  autoware::system_mode_decider::DefaultPlugin, autoware::system_mode_decider::Plugin)
