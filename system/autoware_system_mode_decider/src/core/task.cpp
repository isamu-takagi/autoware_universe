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

#include "task.hpp"

namespace autoware::system_mode_decider
{

void NoneTask::execute(Interface &)
{
  // Do nothing
}

bool NoneTask::timeout(Interface &)
{
  return false;
}

bool NoneTask::expects(const GateStatus &) const
{
  return false;
}

void GateTask::execute(Interface & interface)
{
  if (stamp_) return;
  stamp_ = interface.now();
  interface.change_gate_status(target_);
}

bool GateTask::timeout(Interface & interface)
{
  if (!stamp_) return false;
  return 3.0 < (interface.now() - stamp_.value()).seconds();
}

bool GateTask::expects(const GateStatus & status) const
{
  return target_.type == status.type && target_.id == status.id;
}

}  // namespace autoware::system_mode_decider
