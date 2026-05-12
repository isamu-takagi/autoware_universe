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

from collections import defaultdict

from autoware_driving_mode_msgs.msg import DrivingModeStatus
from autoware_driving_mode_msgs.msg import DrivingModeStatusItem
from autoware_driving_mode_tools.utils import default_qos
from python_qt_binding import QtCore
from python_qt_binding import QtWidgets


def centered_label(text):
    label = QtWidgets.QLabel(text)
    label.setAlignment(QtCore.Qt.AlignCenter)
    return label


class DrivingModeControl(QtWidgets.QWidget):
    def __init__(self, node, modes):
        super().__init__()
        self.clock = node.get_clock()
        self.status = {}
        self.buttons = defaultdict(dict)

        self.create_widget(modes)
        self.timer = node.create_timer(0.5, self.on_timer)
        self.publisher = node.create_publisher(
            DrivingModeStatus, "/system/driving_mode/status", default_qos(1)
        )
        self.subscription = node.create_subscription(
            DrivingModeStatus,
            "/system/driving_mode_manager/debug/driving_mode/status",
            self.on_msg,
            default_qos(1),
        )

    def on_msg(self, msg):
        for buttons in self.buttons.values():
            for button in buttons.values():
                button.setStyleSheet("")
        for item in msg.items:
            self.buttons[(item.type, item.status)][item.mode].setStyleSheet(
                "background-color: lightgreen;"
            )

    def on_timer(self):
        self.publish()

    def publish(self):
        items = []
        for (mode, category), status in self.status.items():
            if status is not None:
                items.append(DrivingModeStatusItem(mode=mode, type=category, status=status))
        msg = DrivingModeStatus(stamp=self.clock.now().to_msg(), items=items)
        self.publisher.publish(msg)

    def create_widget(self, modes):
        layout = QtWidgets.QGridLayout()
        layout.setSpacing(0)
        layout.setRowStretch(len(modes) + 1, 1)
        layout.addWidget(QtWidgets.QLabel("Autoware Mode"), 1, 0)
        layout.addWidget(centered_label("Continuable"), 1, 1, 1, 3)
        layout.addWidget(centered_label("Available"), 1, 4, 1, 3)
        layout.addWidget(centered_label("Stable"), 1, 7, 1, 3)
        self.setLayout(layout)
        self.create_all_buttons(DrivingModeStatusItem.CONTINUABLE, layout, 0, 1)
        self.create_all_buttons(DrivingModeStatusItem.AVAILABLE, layout, 0, 4)
        self.create_all_buttons(DrivingModeStatusItem.STABLE, layout, 0, 7)
        for row, (mode, name) in enumerate(modes, start=2):
            layout.addWidget(QtWidgets.QLabel(f"{name} ({mode})"), row, 0)
            self.create_button(DrivingModeStatusItem.CONTINUABLE, mode, layout, row, 1)
            self.create_button(DrivingModeStatusItem.AVAILABLE, mode, layout, row, 4)
            self.create_button(DrivingModeStatusItem.STABLE, mode, layout, row, 7)

    def set_status(self, mode, category, status):
        self.status[(mode, category)] = status
        self.publish()

    def set_all_status(self, category, status):
        for mode, button in self.buttons[(category, status)].items():
            button.setChecked(True)
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
        self.buttons[(category, None)][mode] = button_none
        self.buttons[(category, True)][mode] = button_true
        self.buttons[(category, False)][mode] = button_false

    def create_all_buttons(self, category, layout, row, col):
        button_none = QtWidgets.QPushButton("None")
        button_true = QtWidgets.QPushButton("True")
        button_false = QtWidgets.QPushButton("False")
        button_none.clicked.connect(lambda: self.set_all_status(category, None))
        button_true.clicked.connect(lambda: self.set_all_status(category, True))
        button_false.clicked.connect(lambda: self.set_all_status(category, False))
        buttons = [button_none, button_true, button_false]
        for index, button in enumerate(buttons):
            layout.addWidget(button, row, col + index)
