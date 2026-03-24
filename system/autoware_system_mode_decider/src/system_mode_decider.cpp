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

#include "system_mode_decider.hpp"

#include <memory>
#include <utility>

namespace autoware::system_mode_decider
{

SystemModeDecider::SystemModeDecider(const rclcpp::NodeOptions & options)
: Node("system_mode_decider", options), diag_(this, 0.1)
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  diag_.setHardwareID("none");
}

}  // namespace autoware::system_mode_decider

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::system_mode_decider::SystemModeDecider)
