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

#ifndef CORE__STATUS_HPP_
#define CORE__STATUS_HPP_

#include <autoware_system_mode_decider/status.hpp>
#include <rclcpp/time.hpp>

#include <optional>
#include <unordered_map>
#include <vector>

namespace autoware::system_mode_decider
{

class TimeoutStatus
{
public:
  bool timeout() const;
  bool status() const;
  void update(const rclcpp::Time & now, bool status);
  void update(const rclcpp::Time & now, double timeout);

private:
  std::optional<rclcpp::Time> stamp_ = std::nullopt;
  bool value_ = false;
};

struct SystemModeStatusData
{
  TimeoutStatus available;
  TimeoutStatus stable;
  TimeoutStatus continuable;
};

class SystemModeStatusStore : public SystemModeStatus
{
public:
  explicit SystemModeStatusStore(const std::vector<AutowareMode> & modes);
  void update(const rclcpp::Time & now, double timeout);
  SystemModeStatusData * data(const AutowareMode & mode);

  bool is_available(const AutowareMode & mode) const override;
  bool is_stable(const AutowareMode & mode) const override;
  bool is_continuable(const AutowareMode & mode) const override;

private:
  std::unordered_map<uint32_t, SystemModeStatusData> modes_;
};

}  // namespace autoware::system_mode_decider

#endif  // CORE__STATUS_HPP_
