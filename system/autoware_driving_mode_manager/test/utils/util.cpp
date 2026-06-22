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

#include "util.hpp"

#include "core/init.hpp"
#include "core/main.hpp"
#include "plugin/default.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <utility>

std::unique_ptr<MainLogic> create_main_logic()
{
  auto interface = std::make_unique<MockInterface>();
  auto plugin = std::make_shared<DefaultPlugin>();
  auto init = std::make_unique<ManagerInit>(std::move(interface), plugin);
  for (const auto & mode : init->config_->autoware_modes()) {
    init->on_available_flag(mode, true);
    init->on_active_flag(mode, true);
    init->on_stable_flag(mode, true);
    init->on_continuable_flag(mode, true);
  }
  init->on_trajectory_source(TrajectorySource{0});
  init->on_command_source(CommandSource{0});
  init->on_command_filter(CommandFilter{true});
  init->on_vehicle_control_mode(PlatformMode::kAutoware);
  EXPECT_TRUE(init->is_ready());
  return std::make_unique<ManagerMain>(*init);
}
