# Copyright 2026 The Autoware Contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from autoware_adapi_v1_msgs.msg import OperationModeState
from autoware_system_msgs.srv import ChangeAutowareControl
from autoware_system_msgs.srv import ChangeOperationMode

from .utils import durable_qos


class OperationMode:
    def __init__(self, node):
        self._pub_state = node.create_publisher(
            OperationModeState,
            "/system/operation_mode/state",
            durable_qos(),
        )
        self._srv_operation_mode = node.create_service(
            ChangeOperationMode,
            "/system/operation_mode/change_operation_mode",
            self._on_operation_mode,
        )
        self._srv_autoware_control = node.create_service(
            ChangeAutowareControl,
            "/system/operation_mode/change_autoware_control",
            self._on_autoware_control,
        )

        self._state = OperationModeState()
        self._state.mode = OperationModeState.UNKNOWN
        self._update()

    def _update(self):
        self._pub_state.publish(self._state)

    def _on_operation_mode(self, request: ChangeOperationMode.Request):
        self._state = request.operation_mode
        self._pub_state.publish(OperationModeState(operation_mode=self._state))
        return ChangeOperationMode.Response(success=True)

    def _on_autoware_control(self, request: ChangeAutowareControl.Request):
        self._state = request.autoware_control
        self._pub_state.publish(OperationModeState(operation_mode=self._state))
        return ChangeAutowareControl.Response(success=True)
