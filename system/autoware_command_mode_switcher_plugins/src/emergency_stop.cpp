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

#include "emergency_stop.hpp"

#include <memory>

namespace autoware::command_mode_switcher
{

void EmergencyStopSwitcher::initialize()
{
  cli_operate_ = node_->create_client<OperateMrmSrv>("/system/mrm/emergency_stop/operate");
  sub_status_ = node_->create_subscription<MrmBehaviorStatus>(
    "/system/mrm/emergency_stop/status", 1,
    [this](const MrmBehaviorStatus & msg) { status_ = msg; });

  requesting_ = false;
  status_.state = MrmBehaviorStatus::NOT_AVAILABLE;
}

SourceState EmergencyStopSwitcher::update_source_state(bool request)
{
  operate(request);
  return SourceState{request, !request};
}

MrmState EmergencyStopSwitcher::update_mrm_state()
{
  return MrmState::Normal;
}

void EmergencyStopSwitcher::operate(bool request)
{
  if (requesting_) return;
  if (status_.state == MrmBehaviorStatus::NOT_AVAILABLE) return;
  if (!cli_operate_->service_is_ready()) return;

  const auto req = std::make_shared<OperateMrmSrv::Request>();
  req->stamp = node_->now();
  req->operate = request;

  using SharedFuture = rclcpp::Client<OperateMrmSrv>::SharedFuture;
  requesting_ = true;
  cli_operate_->async_send_request(req, [this](SharedFuture) { requesting_ = false; });
}

}  // namespace autoware::command_mode_switcher

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  autoware::command_mode_switcher::EmergencyStopSwitcher,
  autoware::command_mode_switcher::CommandPlugin)
