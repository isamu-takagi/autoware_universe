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

#ifndef AUTOWARE_SYSTEM_MODE_DECIDER__STATUS_HPP_
#define AUTOWARE_SYSTEM_MODE_DECIDER__STATUS_HPP_

#include "types.hpp"

#include <unordered_map>
#include <vector>

namespace autoware::system_mode_decider
{

struct SystemModeStatusItem
{
  bool sustainable = false;
  bool available = false;
  bool ready = false;
  bool complete = false;
};

class SystemModeStatus
{
public:
  virtual ~SystemModeStatus() = default;
  virtual SystemModeStatusItem get(uint32_t mode) const = 0;
  virtual std::vector<uint32_t> get_available_modes(uint32_t current) const = 0;
};

}  // namespace autoware::system_mode_decider

#endif  // AUTOWARE_SYSTEM_MODE_DECIDER__STATUS_HPP_
