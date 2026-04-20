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

#ifndef AUTOWARE_SYSTEM_MODE_DECIDER__TYPES_HPP_
#define AUTOWARE_SYSTEM_MODE_DECIDER__TYPES_HPP_

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace autoware::system_mode_decider
{

enum class GateType {
  kTrajectoryGate,
  kCommandGate,
  kCommandFilter,
  kVehicleDriver,
};

struct GateStatus
{
  GateType type;
  uint32_t id;
};

enum class OperationMode {
  kUnknown,
  kStop,
  kAutonomous,
  kLocal,
  kRemote,
};

enum class AutowareControl {
  kUnknown,
  kEnable,
  kDisable,
};

enum class MrmRequest {
  kNone,
  kDelegate,
};

struct AutowareMode
{
  uint32_t id;
};

struct PlatformMode
{
  uint32_t id;
};

struct CurrentModes
{
  OperationMode operation_mode;      // request
  AutowareControl autoware_control;  // request
  MrmRequest mrm_request;            // request
  AutowareMode autoware_mode;        // current
  PlatformMode platform_mode;        // current
};

using ModeMapping = std::unordered_map<uint32_t, std::vector<GateStatus>>;

}  // namespace autoware::system_mode_decider

#endif  // AUTOWARE_SYSTEM_MODE_DECIDER__TYPES_HPP_
