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

#ifndef CORE__INIT_HPP_
#define CORE__INIT_HPP_

#include "core/config.hpp"
#include "core/status.hpp"
#include "type/data.hpp"
#include "type/interface.hpp"

#include <autoware_driving_mode_manager/plugin.hpp>

#include <memory>

namespace autoware::driving_mode_manager
{

class ManagerInit : public MainLogic
{
public:
  ManagerInit(std::unique_ptr<Interface> && interface, std::shared_ptr<Plugin> plugin);
  bool is_ready() const;

  void update() override;
  void notify_trajectory_source(const TrajectorySource & source) override;
  void notify_command_source(const CommandSource & source) override;
  void notify_command_filter(const CommandFilter & filter) override;
  void notify_vehicle_control_mode(const PlatformMode & mode) override;
  void on_available_flag(const AutowareMode & mode, bool flag) override;
  void on_stable_flag(const AutowareMode & mode, bool flag) override;
  void on_continuable_flag(const AutowareMode & mode, bool flag) override;
  ServiceResponse change_operation_mode(const OperationMode & operation_mode) override;
  ServiceResponse change_autoware_control(const AutowareControl & autoware_control) override;

  std::unique_ptr<Interface> interface_;
  std::shared_ptr<Plugin> plugin_;
  std::unique_ptr<DrivingModeConfig> config_;
  std::unique_ptr<DrivingModeStatus> status_;
  GateStatusItem gates_;
};

}  // namespace autoware::driving_mode_manager

#endif  // CORE__INIT_HPP_
