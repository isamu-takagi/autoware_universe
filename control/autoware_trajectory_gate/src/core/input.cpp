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

#include "input.hpp"

#include <memory>
#include <utility>

namespace autoware::trajectory_gate
{

TrajectoryInput::TrajectoryInput(uint16_t id, std::unique_ptr<TimeoutDiag> && timeout) : id_(id)
{
  timeout_ = std::move(timeout);
}

void TrajectoryInput::receive(const Trajectory & msg)
{
  timeout_->update();
  TrajectorySender::send(msg);
}

/*
TimeoutDiag * CommandSource::create_diag_task(
  const TimeoutDiag::Params & params, const rclcpp::Clock & clock)
{
  if (timeout_) {
    throw std::logic_error("timeout diag has already been created");
  }
  timeout_ = std::make_unique<TimeoutDiag>(params, clock, "timeout_" + source_name_);
  return timeout_.get();
}
*/

}  // namespace autoware::trajectory_gate
