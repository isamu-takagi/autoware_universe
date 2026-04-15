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

namespace autoware::system_mode_decider
{

AutowareMode DefaultPlugin::decide()
{
  return AutowareMode{0};
};

ModeMapping DefaultPlugin::mapping() const
{
  ModeMapping mapping;

  // Stop mode
  mapping[0] = {};
  mapping[0].emplace_back(GateStatus{GateType::kCommandGate, 11});

  // Auto mode
  mapping[1] = {};
  mapping[1].emplace_back(GateStatus{GateType::kTrajectoryGate, 100});
  mapping[1].emplace_back(GateStatus{GateType::kCommandGate, 12});

  return mapping;
}

}  // namespace autoware::system_mode_decider

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  autoware::system_mode_decider::DefaultPlugin, autoware::system_mode_decider::Plugin)
