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

#ifndef GATE__INTERFACE_HPP_
#define GATE__INTERFACE_HPP_

#include <rclcpp/time.hpp>

#include <functional>

namespace autoware::system_mode_decider
{

class GateInterface
{
public:
  using Callback = std::function<void(uint32_t)>;
  virtual ~GateInterface() = default;
  virtual uint32_t request(const rclcpp::Time & stamp) = 0;
  virtual void register_callback(Callback callback) = 0;
};

}  // namespace autoware::system_mode_decider

#endif  // GATE__INTERFACE_HPP_
