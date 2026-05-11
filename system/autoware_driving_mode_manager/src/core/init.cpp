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

#include "init.hpp"

#include "values.hpp"

#include <rclcpp/logging.hpp>

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace autoware::driving_mode_manager
{

const auto logger = rclcpp::get_logger("Manager");

ManagerInit::ManagerInit(std::unique_ptr<Interface> && interface)
{
  interface_ = std::move(interface);
  interface_->init(this);

  /*
  config_ = std::make_unique<DrivingModeConfig>();
  plugin_ = plugin;
  plugin_->setup(*config_);
  status_ = std::make_unique<DrivingModeStatus>(config_->autoware_modes());
  */
}

bool ManagerInit::is_ready() const
{
  if (!status_->is_ready()) return false;
  return true;
}

void ManagerInit::update()
{
  status_->update(interface_->now(), 1.0);
}

void ManagerInit::notify_trajectory_source(const TrajectorySource & source)
{
  gates_.trajectory_source = source;
}

void ManagerInit::notify_command_source(const CommandSource & source)
{
  gates_.command_source = source;
}

void ManagerInit::notify_command_filter(const CommandFilter & filter)
{
  gates_.command_filter = filter;
}

void ManagerInit::notify_vehicle_control_mode(const PlatformMode & mode)
{
  gates_.platform_mode = mode;
}

void ManagerInit::on_available_flag(const AutowareMode & mode, bool flag)
{
  if (const auto & data = status_->data(mode)) {
    data->available.update(interface_->now(), flag);
  }
}

void ManagerInit::on_stable_flag(const AutowareMode & mode, bool flag)
{
  if (const auto & data = status_->data(mode)) {
    data->stable.update(interface_->now(), flag);
  }
}

void ManagerInit::on_continuable_flag(const AutowareMode & mode, bool flag)
{
  if (const auto & data = status_->data(mode)) {
    data->continuable.update(interface_->now(), flag);
  }
}

ServiceResponse ManagerInit::change_operation_mode(const OperationMode &)
{
  return ServiceResponse{false, "driving mode manager is not ready"};
}

ServiceResponse ManagerInit::change_autoware_control(const AutowareControl &)
{
  return ServiceResponse{false, "driving mode manager is not ready"};
}

}  // namespace autoware::driving_mode_manager
