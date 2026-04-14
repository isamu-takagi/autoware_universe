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

#include "core/mapping.hpp"
#include "core/task.hpp"
#include "type/gate.hpp"
#include "type/interface.hpp"
#include "type/mode.hpp"

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
  void update();
  void notify_gate_status(const GateStatus & status);

  void change_autoware_mode(const AutowareMode & mode);
  void change_platform_mode(const PlatformMode & mode);

private:
  Task none_task_ = Task{GateStatus{GateType::kInvalid, 0}};
  std::queue<Task> tasks_;
  std::unique_ptr<Interface> interface_;
  std::shared_ptr<Plugin> plugin_;
  std::unordered_map<GateType, uint32_t> actual_gate_status_;

  SystemMode target_;
  ModeMapping mapping_;
};

}  // namespace autoware::system_mode_decider

#endif  // CORE__DECIDER_HPP_
