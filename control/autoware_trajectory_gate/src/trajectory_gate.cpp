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

#include "trajectory_gate.hpp"

#include "core/monitor.hpp"
#include "core/publisher.hpp"
#include "core/subscription.hpp"

#include <memory>
#include <utility>

/*
#include "command/builtin.hpp"
#include "command/compatibility.hpp"
#include "command/filter.hpp"
#include "command/publisher.hpp"
#include "command/source.hpp"
#include "command/subscription.hpp"

#include <autoware_vehicle_info_utils/vehicle_info_utils.hpp>

#include <string>
#include <vector>
*/

namespace autoware::trajectory_gate
{

TrajectoryGate::TrajectoryGate(const rclcpp::NodeOptions & options)
: Node("trajectory_gate", options), diag_(this, 0.1)
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  diag_.setHardwareID("none");

  pub_source_ =
    create_publisher<TrajectorySource>("~/source/status", rclcpp::QoS(1).transient_local());
  sub_source_ = create_subscription<TrajectorySource>(
    "~/source/select", rclcpp::QoS(1), std::bind(&TrajectoryGate::on_select_source, this, _1));

  // Create input streams.
  for (const auto id : {100, 200, 300}) {
    using autoware_utils_diagnostics::TimeoutDiag;
    const auto name = "id" + std::to_string(id);

    TimeoutDiag::Params param;
    param.warn_duration_ = 1.0;
    param.error_duration_ = 2.0;

    auto timeout = std::make_unique<TimeoutDiag>(param, *this->get_clock(), name);
    diag_.add(*timeout);

    auto subscription = std::make_unique<TrajectorySubscription>(name, *this);
    auto monitor = std::make_unique<TrajectoryMonitor>(std::move(timeout));

    subscription->set_output(monitor.get());
    monitor->set_output(nullptr);
    selector_.add_input(monitor.get(), id);

    subscriptions_.push_back(std::move(subscription));
    receivers_.push_back(std::move(monitor));
  }

  // Create output stream.
  {
    auto publisher = std::make_unique<TrajectoryPublisher>(*this);
    selector_.set_output(publisher.get());
    receivers_.push_back(std::move(publisher));
  }

  notify_source();

  /*
  // Create ROS interface.
  pub_status_ =
    create_publisher<CommandSourceStatus>("~/source/status", rclcpp::QoS(1).transient_local());
  srv_select_ = create_service<SelectCommandSource>(
    "~/source/select", std::bind(&TrajectoryGate::on_select_source, this, _1, _2));

  selector_ = std::make_unique<CommandSelector>(get_logger());


  TimeoutDiag::Params params;
  params.warn_duration_ = declare_parameter<double>("diag_timeout_warn_duration");
  params.error_duration_ = declare_parameter<double>("diag_timeout_error_duration");

  auto nominal_filter_params = declare_filter_params(*this, "nominal_filter.");
  auto transition_filter_params = declare_filter_params(*this, "transition_filter.");
  {
    const auto info = autoware::vehicle_info_utils::VehicleInfoUtils(*this).getVehicleInfo();
    nominal_filter_params.wheel_base = info.wheel_base_m;
    transition_filter_params.wheel_base = info.wheel_base_m;
  }

  const auto inputs = declare_parameter<std::vector<int>>("inputs");
  for (const auto & input : inputs) {
    if (input == builtin || input == unknown) {
      throw std::invalid_argument("input source '" + std::to_string(input) + "' is reserved");
    }
  }

  // Create shared data buffer.
  const auto prev_control = std::make_shared<Control>();

  // Create command sources.
  {
    const auto get_source_name = [this](const uint16_t input) {
      return declare_parameter<std::string>("inputs_names." + std::to_string(input));
    };

    std::vector<std::unique_ptr<CommandSource>> sources;
    {
      auto source = std::make_unique<BuiltinEmergency>(builtin, "builtin", *this);
      source->set_prev_control(prev_control);
      sources.push_back(std::move(source));
    }
    for (const auto & input : inputs) {
      auto source = std::make_unique<CommandSubscription>(input, get_source_name(input), *this);
      sources.push_back(std::move(source));
    }
    for (auto & source : sources) {
      diag_.add(*source->create_diag_task(params, *get_clock()));
      selector_->add_source(std::move(source));
    }
  }

  // Create command output.
  {
    auto output = std::make_unique<CommandPublisher>(*this);
    output->set_prev_control(prev_control);
    diag_.add(*output->create_diag_task(params, *get_clock()));

    auto filter = std::make_unique<CommandFilter>(std::move(output), *this);
    output_filter_ = filter.get();
    filter->set_nominal_filter_params(nominal_filter_params);
    filter->set_transition_filter_params(transition_filter_params);

    auto compatibility = std::make_unique<Compatibility>(std::move(filter), *this);
    compatibility_ = compatibility.get();
    compatibility->set_prev_control(prev_control);
    selector_->set_output(std::move(compatibility));
  }

  // Select initial command source. Note that the select function calls on_change_source.
  selector_->select_builtin_source(builtin);
  publish_source_status();

  const auto period = rclcpp::Rate(declare_parameter<double>("rate")).period();
  timer_ = rclcpp::create_timer(this, get_clock(), period, [this]() { on_timer(); });
  */
}

void TrajectoryGate::on_select_source(const TrajectorySource & req)
{
  const auto success = selector_.select(req.source);
  notify_source();

  const auto result = success ? "succeeded" : "failed";
  const auto source = std::to_string(req.source);
  RCLCPP_INFO_STREAM(get_logger(), "source select " << result << " (" << source << ")");
}

void TrajectoryGate::notify_source() const
{
  TrajectorySource msg;
  msg.source = selector_.source();
  pub_source_->publish(msg);
}

/*
void TrajectoryGate::on_timer()
{
  selector_->update();
  compatibility_->publish();
  publish_source_status();
}

void TrajectoryGate::on_select_source(
  const SelectCommandSource::Request::SharedPtr req,
  const SelectCommandSource::Response::SharedPtr res)
{
  const auto error = selector_->select(req->source);
  if (!error.empty()) {
    res->status.success = false;
    res->status.message = error;
    RCLCPP_ERROR_STREAM(get_logger(), error);
    return;
  }
  const auto message = "select command source: " + std::to_string(req->source);
  res->status.success = true;
  res->status.message = message;
  RCLCPP_INFO_STREAM(get_logger(), message);

  // Update transition flag if command source is changed.
  output_filter_->set_transition_flag(req->transition);
  publish_source_status();
}

void TrajectoryGate::publish_source_status()
{
  const auto current_source = selector_->get_source();
  const auto transition_flag = output_filter_->get_transition_flag();
  if (current_source_ == current_source && transition_flag_ == transition_flag) {
    return;
  }
  current_source_ = current_source;
  transition_flag_ = transition_flag;

  CommandSourceStatus msg;
  msg.stamp = now();
  msg.source = current_source_;
  msg.transition = transition_flag_;
  pub_status_->publish(msg);
}
*/

}  // namespace autoware::trajectory_gate

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(autoware::trajectory_gate::TrajectoryGate)
