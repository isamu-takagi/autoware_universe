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

#include "decider.hpp"

#include <cstdio>

namespace autoware::system_mode_decider
{

Decider::Decider()
{
  tasks_.push(TrajectorySwitch(100));
  tasks_.push(CommandSwitch(11));
}

bool Decider::ready() const
{
  return init_flag_ == 0x07;
}

void Decider::update_trajectory_source(uint32_t id)
{
  init_flag_ |= 0x01;
  actual_.trajectory = id;
}

void Decider::update_command_source(uint32_t id)
{
  init_flag_ |= 0x02;
  actual_.command = id;
}

void Decider::update_vehicle_source(uint32_t id)
{
  init_flag_ |= 0x04;
  actual_.vehicle = id;
}

void Decider::update()
{
  std::printf("%d %d %d\n", actual_.trajectory, actual_.command, actual_.vehicle);
}

void Decider::change_autoware_mode(uint32_t id)
{
  std::printf("Change Autoware mode to %d\n", id);
}

void Decider::change_platform_mode(uint32_t id)
{
  std::printf("Change Platform mode to %d\n", id);
}

}  // namespace autoware::system_mode_decider
