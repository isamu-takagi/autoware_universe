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


from autoware_system_mode_msgs.msg import TrajectorySource
from autoware_system_mode_tools.utils import durable_qos
from python_qt_binding import QtWidgets
from tier4_system_msgs.msg import CommandSourceStatus


class TrajectoryGateDisplay(QtWidgets.QLabel):
    def __init__(self, node):
        super().__init__("Trajectory: N/A")
        self.subscription = node.create_subscription(
            TrajectorySource, "/planning/trajectory_gate/source/status", self.on_msg, durable_qos(1)
        )

    def on_msg(self, msg):
        self.setText(f"Trajectory: {msg.source}")


class CommandGateDisplay(QtWidgets.QLabel):
    def __init__(self, node):
        super().__init__("Command: N/A")
        self.subscription = node.create_subscription(
            CommandSourceStatus,
            "/control/control_command_gate/source/status",
            self.on_msg,
            durable_qos(1),
        )

    def on_msg(self, msg):
        self.setText(f"Command: {msg.source}")
