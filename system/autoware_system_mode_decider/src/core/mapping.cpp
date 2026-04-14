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

#include "mapping.hpp"

namespace autoware::system_mode_decider
{

ModeMapping::ModeMapping()
{
  // Stop mode
  autoware_[0] = {};
  autoware_[0].emplace_back(GateStatus{GateType::kCommandGate, 11});

  // Auto mode
  autoware_[1] = {};
  autoware_[1].emplace_back(GateStatus{GateType::kTrajectoryGate, 100});
  autoware_[1].emplace_back(GateStatus{GateType::kCommandGate, 12});
}

}  // namespace autoware::system_mode_decider
