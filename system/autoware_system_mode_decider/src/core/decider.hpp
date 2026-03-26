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

#ifndef CORE__DECIDER_HPP_
#define CORE__DECIDER_HPP_

#include "type/modes.hpp"

namespace autoware::system_mode_decider
{

class Decider
{
public:
  bool ready() const;
  void update_trajectory_source(uint32_t id);
  void update_command_source(uint32_t id);
  void update_vehicle_source(uint32_t id);

private:
  GateStatus request_;
  GateStatus target_;
  GateStatus actual_;
};

}  // namespace autoware::system_mode_decider

#endif  // CORE__DECIDER_HPP_
