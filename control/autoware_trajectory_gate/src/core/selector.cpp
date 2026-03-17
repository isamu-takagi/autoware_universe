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

#include "selector.hpp"

#include <memory>

namespace autoware::trajectory_gate
{

class TrajectoryIgnore : public TrajectoryReceiver
{
public:
  void receive(const Trajectory &) override {}
};

TrajectorySelector::TrajectorySelector()
{
  ignore_ = std::make_unique<TrajectoryIgnore>();
  output_ = nullptr;
}

void TrajectorySelector::add_input(TrajectorySender * input, uint32_t source_id)
{
  if (source_id == invalid_source_id) {
    throw std::runtime_error("trajectory input has an invalid source id");
  }

  const auto [iter, success] = inputs_.insert({source_id, input});
  if (!success) {
    throw std::runtime_error("trajectory input already exists: " + std::to_string(source_id));
  }
  input->set_output(ignore_.get());
}

void TrajectorySelector::set_output(TrajectoryReceiver * output)
{
  if (output_) {
    throw std::runtime_error("trajectory output already exists");
  }
  output_ = output;
}

bool TrajectorySelector::select(uint32_t target_id)
{
  for (const auto & [source_id, input] : inputs_) {
    input->set_output(target_id == source_id ? output_ : ignore_.get());
  }
  return inputs_.count(target_id);  // Use contains function after C++20.
}

uint32_t TrajectorySelector::source() const
{
  for (const auto & [source_id, input] : inputs_) {
    if (input->has_output()) {
      return source_id;
    }
  }
  return invalid_source_id;
}

}  // namespace autoware::trajectory_gate
