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

#include "status.hpp"

#include <vector>

namespace autoware::system_mode_decider
{

bool TimeoutStatus::timeout() const
{
  return stamp_ == std::nullopt;
}

bool TimeoutStatus::status() const
{
  return stamp_ ? value_ : false;
}

void TimeoutStatus::update(const rclcpp::Time & now, bool status)
{
  stamp_ = now;
  value_ = status;
}

void TimeoutStatus::update(const rclcpp::Time & now, double timeout)
{
  if (stamp_ && timeout < (now - stamp_.value()).seconds()) {
    stamp_ = std::nullopt;
    value_ = false;
  }
}

SystemModeStatusStore::SystemModeStatusStore(const std::vector<AutowareMode> & modes)
{
  for (const auto & mode : modes) {
    available_[mode.id] = TimeoutStatus();
    stable_[mode.id] = TimeoutStatus();
    continuable_[mode.id] = TimeoutStatus();
  }
}

void SystemModeStatusStore::update(const rclcpp::Time & now, double timeout)
{
  for (auto & [id, status] : available_) status.update(now, timeout);
  for (auto & [id, status] : stable_) status.update(now, timeout);
  for (auto & [id, status] : continuable_) status.update(now, timeout);
}

TimeoutStatus & SystemModeStatusStore::available(const AutowareMode & mode)
{
  return available_.at(mode.id);
}

TimeoutStatus & SystemModeStatusStore::stable(const AutowareMode & mode)
{
  return stable_.at(mode.id);
}

TimeoutStatus & SystemModeStatusStore::continuable(const AutowareMode & mode)
{
  return continuable_.at(mode.id);
}

bool SystemModeStatusStore::is_available(const AutowareMode & mode) const
{
  const auto iter = available_.find(mode.id);
  return iter == available_.end() ? false : iter->second.status();
}

bool SystemModeStatusStore::is_stable(const AutowareMode & mode) const
{
  const auto iter = stable_.find(mode.id);
  return iter == stable_.end() ? false : iter->second.status();
}

bool SystemModeStatusStore::is_continuable(const AutowareMode & mode) const
{
  const auto iter = continuable_.find(mode.id);
  return iter == continuable_.end() ? false : iter->second.status();
}

}  // namespace autoware::system_mode_decider
