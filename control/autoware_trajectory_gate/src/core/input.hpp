// Copyright 2025 The Autoware Contributors
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

#ifndef CORE__INPUT_HPP_
#define CORE__INPUT_HPP_

#include "common/timeout_diagnostics.hpp"
#include "interface.hpp"

#include <memory>
#include <string>

namespace autoware::trajectory_gate
{

class TrajectoryInput
{
public:
  explicit TrajectoryInput(uint16_t id);

  uint16_t id() const { return id_; }
  void set_output(TrajectoryOutput * output) { output_ = output; }

  // bool is_timeout() const { return timeout_->is_error(); }

protected:
  void send(const Trajectory & msg);

private:
  const uint16_t id_;
  TrajectoryOutput * output_;
  // std::unique_ptr<TimeoutDiag> timeout_;
};

}  // namespace autoware::trajectory_gate

#endif  // CORE__INPUT_HPP_
