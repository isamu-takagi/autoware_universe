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

import copy

from autoware_adapi_v1_msgs.msg import OperationModeState
from autoware_mock_common.utils import durable_qos
from autoware_system_msgs.srv import ChangeAutowareControl
from autoware_system_msgs.srv import ChangeOperationMode
from rclpy.node import Node


class OperationMode:
    def __init__(self, node: Node):
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

        self._clock = node.get_clock()
        # self._timer = node.create_timer(1.0, self._update)
        self._state = OperationModeState()
        self._state.mode = OperationModeState.UNKNOWN
        self._state.is_autoware_control_enabled = True
        self._state.is_in_transition = False
        self._state.is_stop_mode_available = True
        self._state.is_autonomous_mode_available = True
        self._state.is_local_mode_available = False
        self._state.is_remote_mode_available = False
        self._update()

    def _update(self):
        state = copy.deepcopy(self._state)
        state.stamp = self._clock.now().to_msg()
        self._pub_state.publish(state)

    def _on_operation_mode(self, req, res):
        mode = self.__class__._mode_dict.get(req.mode)
        if mode is None:
            res.status.success = False
            res.status.message = "unknown mode"
            return res
        else:
            self._state.mode = mode
            self._update()
            res.status.success = True
            return res

    def _on_autoware_control(self, req, res):
        pass

    _mode_dict = {
        ChangeOperationMode.Request.STOP: OperationModeState.STOP,
        ChangeOperationMode.Request.AUTONOMOUS: OperationModeState.AUTONOMOUS,
        ChangeOperationMode.Request.LOCAL: OperationModeState.LOCAL,
        ChangeOperationMode.Request.REMOTE: OperationModeState.REMOTE,
    }
