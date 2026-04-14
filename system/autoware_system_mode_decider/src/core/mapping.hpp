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

#ifndef CORE__MAPPING_HPP_
#define CORE__MAPPING_HPP_

#include "type/gate.hpp"
#include "type/mode.hpp"

#include <unordered_map>
#include <vector>

namespace autoware::system_mode_decider
{

class ModeMapping
{
public:
  ModeMapping();
  const auto & from(AutowareMode mode) const { return autoware_.at(mode.id); }

private:
  std::unordered_map<uint32_t, std::vector<GateStatus>> autoware_;
};

}  // namespace autoware::system_mode_decider

#endif  // CORE__MAPPING_HPP_
