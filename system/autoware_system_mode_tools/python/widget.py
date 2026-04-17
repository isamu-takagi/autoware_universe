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
from autoware_system_mode_msgs.msg import SystemModeStatusItem
from autoware_system_mode_msgs.msg import TrajectorySource
from python_qt_binding import QtWidgets
from tier4_system_msgs.msg import CommandSourceStatus

from .utils import default_qos
from .utils import durable_qos


class MainWidget(QtWidgets.QWidget):
    def __init__(self, node):
        super().__init__()
        modes = [
            (101, "Stop"),
            (102, "Autonomous"),
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
        self.clock = node.get_clock()
        self.status = {}

        self.create_widget(modes)
        self.timer = node.create_timer(0.5, self.on_timer)
        self.publisher = node.create_publisher(
            SystemModeStatus, "/system/driving_mode/status", default_qos(1)
        )

    def on_msg(self, msg):
        print(msg.source)

    def on_timer(self):
        self.publish()

    def publish(self):
        items = []
        for (mode, category), status in self.status.items():
            if status is not None:
                items.append(SystemModeStatusItem(mode=mode, type=category, status=status))
        msg = SystemModeStatus(stamp=self.clock.now().to_msg(), items=items)
        self.publisher.publish(msg)

    def create_widget(self, modes):
        layout = QtWidgets.QGridLayout()
        layout.setSpacing(0)
        layout.setRowStretch(len(modes) + 1, 1)
        layout.addWidget(QtWidgets.QLabel("Autoware Mode"), 0, 0)
        layout.addWidget(QtWidgets.QLabel("Available"), 0, 1, 1, 3)
        layout.addWidget(QtWidgets.QLabel("Continuable"), 0, 4, 1, 3)
        self.setLayout(layout)
        for row, (mode, name) in enumerate(modes, start=1):
            layout.addWidget(QtWidgets.QLabel(f"{name} ({mode})"), row, 0)
            self.create_button(SystemModeStatusItem.AVAILABLE, mode, layout, row, 1)
            self.create_button(SystemModeStatusItem.CONTINUABLE, mode, layout, row, 4)

    def set_status(self, mode, category, status):
        self.status[(mode, category)] = status
        self.publish()

    def create_button(self, category, mode, layout, row, col):
        button_none = QtWidgets.QPushButton("None")
        button_true = QtWidgets.QPushButton("True")
        button_false = QtWidgets.QPushButton("False")
        button_group = QtWidgets.QButtonGroup(self)
        button_none.clicked.connect(lambda: self.set_status(mode, category, None))
        button_true.clicked.connect(lambda: self.set_status(mode, category, True))
        button_false.clicked.connect(lambda: self.set_status(mode, category, False))
        buttons = [button_none, button_true, button_false]
        for index, button in enumerate(buttons):
            button_group.addButton(button)
            button.setCheckable(True)
            layout.addWidget(button, row, col + index)


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
