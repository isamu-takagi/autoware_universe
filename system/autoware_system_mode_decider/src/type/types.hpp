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

#ifndef TYPE__TYPES_HPP_
#define TYPE__TYPES_HPP_

#include <rclcpp/time.hpp>

namespace autoware::system_mode_decider
{

template <typename Type, typename Time = rclcpp::Time>
class TimerValue
{
public:
  TimerValue() { timeout_ = true; }

  void update(const Type & value, const Time & stamp)
  {
    timeout_ = false;
    value_ = value;
    stamp_ = stamp;
  }

  void update(const Time & stamp, const double timeout) { stamp_ = stamp; }

  Type value() const { return value_; }
  Time stamp() const { return stamp_; }

private:
  bool timeout_;
  Type value_;
  Time stamp_;
};

}  // namespace autoware::system_mode_decider

#endif  // TYPE__TYPES_HPP_
