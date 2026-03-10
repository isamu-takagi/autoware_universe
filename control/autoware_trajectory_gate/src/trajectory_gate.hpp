//  Copyright 2025 The Autoware Contributors
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef TRAJECTORY_GATE_HPP_
#define TRAJECTORY_GATE_HPP_

#include "core/input.hpp"

#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/rclcpp.hpp>

/*
#include "command/compatibility.hpp"
#include "command/filter.hpp"
#include "command/interface.hpp"
#include "command/selector.hpp"

#include <autoware_command_mode_types/sources.hpp>

#include <tier4_system_msgs/msg/command_source_status.hpp>
#include <tier4_system_msgs/srv/select_command_source.hpp>

#include <memory>
#include <string>
*/

namespace autoware::trajectory_gate
{

class TrajectoryGate : public rclcpp::Node
{
public:
  explicit TrajectoryGate(const rclcpp::NodeOptions & options);

private:
  diagnostic_updater::Updater diag_;

  /*
    static constexpr uint16_t unknown = autoware::command_mode_types::sources::unknown;
    static constexpr uint16_t builtin = autoware::command_mode_types::sources::builtin;
    using CommandSourceStatus = tier4_system_msgs::msg::CommandSourceStatus;
    using SelectCommandSource = tier4_system_msgs::srv::SelectCommandSource;

    void publish_source_status();
    void on_timer();
    void on_select_source(
      const SelectCommandSource::Request::SharedPtr req,
      const SelectCommandSource::Response::SharedPtr res);

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<CommandSourceStatus>::SharedPtr pub_status_;
    rclcpp::Service<SelectCommandSource>::SharedPtr srv_select_;


    std::unique_ptr<CommandSelector> selector_;
    CommandFilter * output_filter_;
    Compatibility * compatibility_;

    uint16_t current_source_ = 0;
    bool transition_flag_ = false;
  */
};

}  // namespace autoware::trajectory_gate

#endif  // TRAJECTORY_GATE_HPP_
