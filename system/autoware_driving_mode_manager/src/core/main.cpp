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

#include "main.hpp"

#include "values.hpp"

#include <rclcpp/logging.hpp>

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace autoware::driving_mode_manager
{

const auto logger = rclcpp::get_logger("ManagerMain");
constexpr AutowareMode unknown_mode = AutowareMode{0};

template <typename ModeIterable>
void print_modes(const std::string & title, const ModeIterable & modes)
{
  std::string text;
  for (const auto & mode : modes) {
    text = text + " " + std::to_string(mode.id);
  }
  RCLCPP_INFO_STREAM(logger, title << ":" << text);
}

ManagerMain::ManagerMain(ManagerInit & init)
{
  interface_ = std::move(init.interface_);
  interface_->init(this);

  plugin_ = std::move(init.plugin_);
  config_ = std::move(init.config_);
  status_ = std::move(init.status_);
  mrm_states_ = std::move(init.mrm_states_);

  gates_.status = init.gates();
  gates_.expect = init.gates();

  request_.operation_mode = config_->to_autoware_mode(OperationMode::kStop);
  request_.platform_mode = init.platform_mode.value();
  request_.mrm_strategy = MrmStrategy::kNone;
  request_.mrm_behavior = unknown_mode;
  request_.autoware_mode = unknown_mode;
}

bool ManagerMain::is_ready() const
{
  if (!status_->is_ready()) return false;
  return true;
}

void ManagerMain::update()
{
  // Detect status timeout.
  status_->update(interface_->now(), 1.0);

  // List available modes.
  AutowareModeSet availables;
  for (const auto & mode : config_->autoware_modes()) {
    if (temporary_unavailable_modes_.count(mode) == 0) {
      if (mode.id != request_.autoware_mode.id) {
        if (status_->is_available(mode)) availables.insert(mode);
      } else {
        if (status_->is_continuable(mode)) availables.insert(mode);
      }
    }
  }

  // TODO(isamu-takagi): Check frequently mode change.
  change_autoware_mode(plugin_->decide(request_, availables));
  execute_tasks();
  publish_operation_mode();
  publish_mrm_state();
  publish_driving_mode_request();
  publish_debug();
}

void ManagerMain::publish_operation_mode() const
{
  const auto is_available = [this](const OperationMode & mode) {
    return status_->is_available(config_->to_autoware_mode(mode));
  };

  OperationModeState state;
  state.mode = config_->to_operation_mode(request_.operation_mode);
  state.is_autoware_control_enabled = (request_.platform_mode != PlatformMode::kManual);
  state.is_in_transition = !tasks_.empty();
  state.is_stop_mode_available = is_available(OperationMode::kStop);
  state.is_autonomous_mode_available = is_available(OperationMode::kAutonomous);
  state.is_local_mode_available = is_available(OperationMode::kLocal);
  state.is_remote_mode_available = is_available(OperationMode::kRemote);
  interface_->publish_operation_mode(state);
}

void ManagerMain::publish_mrm_state() const
{
  const auto behavior = config_->to_mrm_behavior(request_.autoware_mode);
  if (behavior) {
    const auto iter = mrm_states_.find(request_.autoware_mode);
    const auto state = iter != mrm_states_.end() ? iter->second : MrmState::State::kUnknown;
    interface_->publish_mrm_state(MrmState{state, behavior.value()});
  } else {
    interface_->publish_mrm_state(MrmState{MrmState::State::kNormal, MrmState::NoneBehavior});
  }
}

void ManagerMain::publish_driving_mode_request() const
{
  interface_->publish_driving_mode_request(request_.autoware_mode);
}

void ManagerMain::publish_debug() const
{
  DebugStatus debug;
  for (const auto & mode : config_->autoware_modes()) {
    DebugStatus::Flag flag;
    flag.available = status_->is_available(mode);
    flag.active = status_->is_active(mode);
    flag.stable = status_->is_stable(mode);
    flag.continuable = status_->is_continuable(mode);
    debug.flags[mode] = flag;
  }
  interface_->publish_debug(debug);
  interface_->publish_debug(request_);
}

void ManagerMain::on_trajectory_source(const TrajectorySource & source)
{
  if (gates_.expect.trajectory_source != source) {
    RCLCPP_WARN_STREAM(logger, "trajectory source override: " << source.id);
  }
  gates_.status.trajectory_source = source;
  gates_.expect.trajectory_source = source;
  execute_tasks();
}

void ManagerMain::on_command_source(const CommandSource & source)
{
  if (gates_.expect.command_source != source) {
    RCLCPP_WARN_STREAM(logger, "command source override: " << source.id);
  }
  gates_.status.command_source = source;
  gates_.expect.command_source = source;
  execute_tasks();
}

void ManagerMain::on_command_filter(const CommandFilter & filter)
{
  if (gates_.expect.command_filter != filter) {
    RCLCPP_WARN_STREAM(logger, "command filter override: " << filter.flag);
  }
  gates_.status.command_filter = filter;
  gates_.expect.command_filter = filter;
  execute_tasks();
}

void ManagerMain::on_vehicle_control_mode(const PlatformMode & mode)
{
  if (gates_.expect.platform_mode != mode) {
    RCLCPP_WARN_STREAM(logger, "platform mode override: " << to_string(mode));
    request_.platform_mode = mode;
    tasks_.clear_platform_tasks();
    tasks_.clear_finalize_tasks();
    tasks_.add_finalize_tasks(std::make_unique<CommandFilterTask>(CommandFilter{false}));
  }
  gates_.status.platform_mode = mode;
  gates_.expect.platform_mode = mode;
  execute_tasks();
}

void ManagerMain::on_available_flag(const AutowareMode & mode, bool flag)
{
  if (const auto & data = status_->data(mode)) {
    data->available.update(interface_->now(), flag);
  }
}

void ManagerMain::on_stable_flag(const AutowareMode & mode, bool flag)
{
  if (const auto & data = status_->data(mode)) {
    data->stable.update(interface_->now(), flag);
  }
}

void ManagerMain::on_continuable_flag(const AutowareMode & mode, bool flag)
{
  if (const auto & data = status_->data(mode)) {
    data->continuable.update(interface_->now(), flag);
  }
}

void ManagerMain::on_mrm_state(const AutowareMode & mode, const MrmState::State & state)
{
  mrm_states_[mode] = state;
}

ServiceResponse ManagerMain::change_mrm_request(const MrmRequest & request)
{
  if (request.strategy == MrmStrategy::kNone || request.strategy == MrmStrategy::kDelegate) {
    request_.mrm_strategy = request.strategy;
    request_.mrm_behavior = unknown_mode;
    return ServiceResponse{true, ""};
  }
  if (request.strategy == MrmStrategy::kBehavior) {
    const auto behavior = config_->to_autoware_mode(request.behavior);
    if (!behavior) {
      return ServiceResponse{false, "unknown behavior"};
    }
    request_.mrm_strategy = request.strategy;
    request_.mrm_behavior = behavior.value();
    return ServiceResponse{true, ""};
  }
  return ServiceResponse{false, "unknown strategy"};
}

ServiceResponse ManagerMain::change_operation_mode(const OperationMode & operation_mode)
{
  if (!tasks_.interruptible()) {
    return ServiceResponse{false, "mode transition is in progress"};
  }

  const auto mode = config_->to_autoware_mode(operation_mode);
  if (!status_->is_available(mode)) {
    return ServiceResponse{false, "operation mode is not available"};
  }

  request_.operation_mode = mode;
  return ServiceResponse{true, ""};
}

ServiceResponse ManagerMain::change_autoware_control(const AutowareControl & autoware_control)
{
  const auto platform_mode = to_platform_mode(autoware_control);

  // If disable, request the manual mode immediately.
  if (platform_mode == PlatformMode::kManual) {
    RCLCPP_INFO_STREAM(logger, "accept autoware control disable");
    request_.platform_mode = platform_mode;
    interface_->change_platform_mode(platform_mode);
    return ServiceResponse{true, ""};
  }

  if (!tasks_.interruptible()) {
    return ServiceResponse{false, "mode transition is in progress"};
  }

  // The check target is the normal behavior, operation mode. MRM is not included.
  const auto mode = request_.operation_mode;
  if (!status_->is_available(mode)) {
    return ServiceResponse{false, "operation mode is not available"};
  }
  if (!status_->is_active(mode)) {
    return ServiceResponse{false, "operation mode is not activated"};
  }

  tasks_.clear_platform_tasks();
  tasks_.add_platform_tasks(std::make_unique<CommandFilterTask>(CommandFilter{true}));
  tasks_.add_platform_tasks(std::make_unique<PlatformModeTask>(PlatformMode::kAutoware));

  tasks_.clear_finalize_tasks();
  tasks_.add_finalize_tasks(std::make_unique<WaitModeStableTask>(mode));
  tasks_.add_finalize_tasks(std::make_unique<CommandFilterTask>(CommandFilter{false}));

  request_.platform_mode = platform_mode;
  return ServiceResponse{true, ""};
}

void ManagerMain::change_autoware_mode(const AutowareMode & mode)
{
  AutowareMode & prev = request_.autoware_mode;
  if (prev.id == mode.id) {
    return;
  }
  if (!config_->exists(mode)) {
    RCLCPP_ERROR_STREAM(logger, "decision logic returns unknown mode: " << mode.id);
    return;
  }
  RCLCPP_INFO_STREAM(logger, "Change Autoware Mode: " << prev.id << " -> " << mode.id);
  prev = mode;

  const auto gates = config_->gates(mode);
  tasks_.clear_autoware_tasks();
  tasks_.add_autoware_tasks(std::make_unique<CommandFilterTask>(CommandFilter{true}));
  tasks_.add_autoware_tasks(std::make_unique<WaitModeActiveTask>(mode));
  if (gates.trajectory) {
    tasks_.add_autoware_tasks(std::make_unique<TrajectorySourceTask>(gates.trajectory.value()));
  }
  if (gates.command) {
    tasks_.add_autoware_tasks(std::make_unique<CommandSourceTask>(gates.command.value()));
  }

  tasks_.clear_finalize_tasks();
  tasks_.add_finalize_tasks(std::make_unique<WaitModeStableTask>(mode));
  tasks_.add_finalize_tasks(std::make_unique<CommandFilterTask>(CommandFilter{false}));
}

void ManagerMain::execute_tasks()
{
  while (!tasks_.empty()) {
    const auto result = tasks_.get()->execute(*interface_, gates_, *status_);
    switch (result) {
      case TaskResult::kFinished:
        RCLCPP_INFO_STREAM(logger, tasks_.get()->describe() << ": finished");
        tasks_.pop();
        break;
      case TaskResult::kRunning:
        RCLCPP_INFO_STREAM(logger, tasks_.get()->describe() << ": running");
        return;
      case TaskResult::kTimeout:
        RCLCPP_WARN_STREAM(logger, tasks_.get()->describe() << ": timeout");
        temporary_unavailable_modes_.insert(request_.autoware_mode);
        return;
      default:
        throw std::logic_error("invalid task result");
    }
  }
}

}  // namespace autoware::driving_mode_manager
