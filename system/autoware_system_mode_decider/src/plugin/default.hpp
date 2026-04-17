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

#ifndef PLUGIN__DEFAULT_HPP_
#define PLUGIN__DEFAULT_HPP_

#include <autoware_system_mode_decider/plugin.hpp>

namespace autoware::system_mode_decider
{

class DefaultPlugin : public Plugin
{
public:
  AutowareMode decide(const CurrentModes & modes, const SystemModeStatus & status) override;
  ModeMapping mapping() const override;

private:
  AutowareMode from_operation_mode(const OperationMode & operation_mode) const;
};

}  // namespace autoware::system_mode_decider

#endif  // PLUGIN__DEFAULT_HPP_
