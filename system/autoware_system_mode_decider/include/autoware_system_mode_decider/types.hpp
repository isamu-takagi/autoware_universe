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
#include <unordered_set>
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

struct TrajectorySource
{
  uint32_t id;
  bool operator==(const TrajectorySource & another) const { return id == another.id; }
  bool operator!=(const TrajectorySource & another) const { return id != another.id; }
};

struct CommandSource
{
  uint32_t id;
  bool operator==(const CommandSource & another) const { return id == another.id; }
  bool operator!=(const CommandSource & another) const { return id != another.id; }
};

struct AutowareMode
{
  uint32_t id;
  bool operator==(const AutowareMode & another) const { return id == another.id; }
  bool operator!=(const AutowareMode & another) const { return id != another.id; }
};

enum class PlatformMode {
  kUnknown,
  kManual,
  kAutoware,
  kAutowareSteering,
  kAutowareVelocity,
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

struct CurrentModes
{
  AutowareControl autoware_control;      // request
  MrmRequest mrm_request;                // request
  AutowareMode operation_autoware_mode;  // request
  AutowareMode autoware_mode;            // current
};

using AutowareModeSet = std::unordered_set<AutowareMode>;
using ModeMapping = std::unordered_map<AutowareMode, std::vector<GateStatus>>;

}  // namespace autoware::system_mode_decider

namespace std
{

template <>
struct hash<autoware::system_mode_decider::AutowareMode>
{
  size_t operator()(const autoware::system_mode_decider::AutowareMode & k) const
  {
    return hash<uint32_t>{}(k.id);
  }
};

}  // namespace std

#endif  // AUTOWARE_SYSTEM_MODE_DECIDER__TYPES_HPP_
