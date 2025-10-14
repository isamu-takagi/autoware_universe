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

#ifndef PATH_WITH_LANE_ID_HPP_
#define PATH_WITH_LANE_ID_HPP_

#include "interface.hpp"

#include <autoware_evaluation_msgs/msg/path_with_lane_id_v1.hpp>
#include <autoware_internal_planning_msgs/msg/path_with_lane_id.hpp>

namespace autoware::evaluation_adapter
{

class PathWithLaneIdAdapterV1 : public rclcpp::Node
{
public:
  explicit PathWithLaneIdAdapterV1(const rclcpp::NodeOptions & options);

private:
  using InternalMessage = autoware_internal_planning_msgs::msg::PathWithLaneId;
  using ExternalMessage = autoware_evaluation_msgs::msg::PathWithLaneIdV1;
  MessageAdapter<InternalMessage, ExternalMessage> adapter_;
};

}  // namespace autoware::evaluation_adapter

#endif  // PATH_WITH_LANE_ID_HPP_
