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

#include <utility>

//
#include <cstdio>

namespace autoware::system_mode_decider
{

Decider::Decider(std::unique_ptr<Interface> && interface)
{
  interface_ = std::move(interface);

  tasks_.push(Task{GateStatus{GateType::kTrajectoryGate, 100}});
  tasks_.push(Task{GateStatus{GateType::kCommandGate, 11}});
}

void Decider::update()
{
  // std::printf("%d %d %d\n", actual_.trajectory, actual_.command, actual_.vehicle);

  Task & task = tasks_.empty() ? none_task_ : tasks_.front();
  (void)task;

  // タスク実行
  // 未実行：変更要求を実行して「要求中」に遷移
  // 要求中：タイムアウトしたら「再決定」を発火？
}

void Decider::notify_gate_status(const GateStatus & status)
{
  Task & task = tasks_.empty() ? none_task_ : tasks_.front();

  if (task.expects(status)) {
    tasks_.pop();
    // update();
  } else {
    // Override detected !!
  }
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
