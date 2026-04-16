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

from autoware_system_mode_msgs.msg import SystemModeStatus
from autoware_system_mode_msgs.msg import TrajectorySource
from python_qt_binding import QtWidgets
from tier4_system_msgs.msg import CommandSourceStatus

from .utils import default_qos
from .utils import durable_qos


class MainWidget(QtWidgets.QWidget):
    def __init__(self, node):
        super().__init__()
        modes = [
            (1, "Stop"),
            (2, "Autonomous"),
        ]
        self.modes_control = DrivingModeControl(node, modes)
        self.status_trajectory = TrajectoryGateDisplay(node)
        self.status_command = CommandGateDisplay(node)

        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.modes_control)
        layout.addWidget(self.status_trajectory)
        layout.addWidget(self.status_command)
        self.setLayout(layout)


class DrivingModeControl(QtWidgets.QWidget):
    def __init__(self, node, modes):
        super().__init__()
        self.buttons = {}

        self.create_widget(modes)
        self.timer = node.create_timer(0.5, self.on_timer)
        self.publisher = node.create_publisher(
            SystemModeStatus, "/system/driving_mode/status", default_qos(1)
        )

    def on_msg(self, msg):
        print(msg.source)

    def on_timer(self):
        self.publish()

    def on_clicked(self, mode, status):
        self.publish()

    def publish(self):
        msg = SystemModeStatus()
        self.publisher.publish(msg)

    def create_widget(self, modes):
        layout = QtWidgets.QGridLayout()
        self.setLayout(layout)
        for row, (mode, name) in enumerate(modes):
            layout.addWidget(QtWidgets.QLabel(f"{name} ({mode})"), row, 0)
            self.create_button(mode, layout, row)

    def create_button(self, mode, layout, row):
        button_none = QtWidgets.QPushButton("None")
        button_true = QtWidgets.QPushButton("True")
        button_false = QtWidgets.QPushButton("False")
        button_group = QtWidgets.QButtonGroup(self)
        buttons = [button_none, button_true, button_false]
        for col, button in enumerate(buttons, start=1):
            button_group.addButton(button)
            button.setCheckable(True)
            button.clicked.connect(self.on_clicked)
            layout.addWidget(button, row, col)
        self.buttons[mode] = buttons


class TrajectoryGateDisplay(QtWidgets.QLabel):
    def __init__(self, node):
        super().__init__("Trajectory: N/A")
        self.subscription = node.create_subscription(
            TrajectorySource, "/planning/trajectory_gate/source/status", self.on_msg, durable_qos(1)
        )

    def on_msg(self, msg):
        print(msg.source)
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
        print(msg.source)
        self.setText(f"Command: {msg.source}")
