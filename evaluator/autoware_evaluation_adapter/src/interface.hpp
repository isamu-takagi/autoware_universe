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

#ifndef INTERFACE_HPP_
#define INTERFACE_HPP_

#include <rclcpp/rclcpp.hpp>

#include <string>

namespace autoware::evaluation_adapter
{

template <class InternalMessage, class ExternalMessage>
class MessageAdapter : public rclcpp::Node
{
public:
  MessageAdapter(const std::string & name, const rclcpp::NodeOptions & options);

private:
  virtual ExternalMessage convert(const InternalMessage & msg) const = 0;
  rclcpp::Publisher<ExternalMessage>::SharedPtr pub_;
  rclcpp::Subscription<InternalMessage>::SharedPtr sub_;
};

}  // namespace autoware::evaluation_adapter

#endif  // INTERFACE_HPP_
