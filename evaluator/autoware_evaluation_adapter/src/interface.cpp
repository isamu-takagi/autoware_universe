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

#include "interface.hpp"

#include <string>

namespace autoware::evaluation_adapter
{

MessageAdapter::MessageAdapter(const std::string & name, const rclcpp::NodeOptions & options)
: Node(name, options)
{
  const auto callback = [this](const InternalMessage & msg) { pub_->publish(convert(msg)); };
  pub_ = create_publisher<ExternalMessage>(name, 1);
  sub_ = create_subscription<InternalMessage>(name, 1, callback);
}

}  // namespace autoware::evaluation_adapter

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::evaluation_adapter::MessageAdapter)
