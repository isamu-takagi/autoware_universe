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

#include "core/status.hpp"
#include "core/task.hpp"
#include "type/interface.hpp"

#include <autoware_system_mode_decider/plugin.hpp>

#include <memory>
#include <queue>
#include <unordered_map>

namespace autoware::system_mode_decider
{

class DeciderLogic
{
public:
  DeciderLogic();
};

class Decider
{
public:
  Decider(std::unique_ptr<Interface> && interface, std::shared_ptr<Plugin> plugin);
  SystemModeStatusStore & access_status();
  void update();
  void notify_gate_status(const GateStatus & status);
  void request_autoware_mode(const AutowareMode & mode);

private:
  void update_autoware_mode(const AutowareMode & mode);
  void update_platform_mode(const PlatformMode & mode);

  std::unique_ptr<Interface> interface_;
  std::shared_ptr<Plugin> plugin_;
  ModeMapping mapping_;
  SystemModeStatusStore driving_mode_status_;

  Task none_task_ = Task{GateStatus{GateType::kInvalid, 0}};
  std::queue<Task> tasks_;
  std::unordered_map<GateType, uint32_t> actual_gate_status_;

  AutowareMode autoware_;
  PlatformMode platform_;
};

}  // namespace autoware::system_mode_decider

#endif  // CORE__DECIDER_HPP_
