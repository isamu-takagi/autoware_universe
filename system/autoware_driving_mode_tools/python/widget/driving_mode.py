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

from enum import Enum

from autoware_driving_mode_manager.msg import DebugModeFlag
from autoware_driving_mode_tools.utils import default_qos
from python_qt_binding import QtCore
from python_qt_binding import QtWidgets
from tier4_system_msgs.msg import DrivingModeFlag
from tier4_system_msgs.msg import DrivingModeFlagItem


def centered_label(text):
    label = QtWidgets.QLabel(text)
    label.setAlignment(QtCore.Qt.AlignCenter)
    return label


class FlagType(Enum):
    Available = 1
    Stable = 2
    Continuable = 3


class DrivingModeControl(QtWidgets.QWidget):
    def __init__(self, node, modes):
        super().__init__()
        self.modes = [mode for mode, name in modes]
        self.clock = node.get_clock()
        self.button = {}
        self.status = {}
        self.flags = {}
        self.timer = node.create_timer(0.5, self.on_timer)

        self.publishers = {}
        self.publishers[FlagType.Continuable] = node.create_publisher(
            DrivingModeFlag, "/system/driving_mode/continuable", default_qos(1)
        )
        self.publishers[FlagType.Available] = node.create_publisher(
            DrivingModeFlag, "/system/driving_mode/available", default_qos(1)
        )
        self.publishers[FlagType.Stable] = node.create_publisher(
            DrivingModeFlag, "/system/driving_mode/stable", default_qos(1)
        )
        self.subscription = node.create_subscription(
            DebugModeFlag,
            "/system/driving_mode_manager/debug/status",
            self.on_msg,
            default_qos(1),
        )
        self.create_widget(modes)

    def on_msg(self, msg):
        for item in zip(msg.mode, msg.available, msg.stable, msg.continuable):
            mode, available, stable, continuable = item
            text = ""
            text += "C" if continuable else "-"
            text += "A" if available else "-"
            text += "S" if stable else "-"
            self.flags[mode].setText(text)

    def on_timer(self):
        self.publish(FlagType.Continuable)
        self.publish(FlagType.Available)
        self.publish(FlagType.Stable)

    def publish(self, flag):
        items = []
        for mode in self.modes:
            status = self.status.get((mode, flag))
            if status is not None:
                items.append(DrivingModeFlagItem(mode=mode, flag=status))
        msg = DrivingModeFlag(stamp=self.clock.now().to_msg(), items=items)
        self.publishers[flag].publish(msg)

    def set_status(self, mode, flag, status):
        self.status[(mode, flag)] = status
        self.publish(flag)

    def set_all_status(self, flag, status):
        for mode in self.modes:
            self.button[(mode, flag, status)].setChecked(True)
            self.status[(mode, flag)] = status
        self.publish(flag)

    def create_button(self, flag, mode, layout, row, col):
        button_none = QtWidgets.QPushButton("None")
        button_true = QtWidgets.QPushButton("True")
        button_false = QtWidgets.QPushButton("False")
        button_group = QtWidgets.QButtonGroup(self)
        button_none.clicked.connect(lambda: self.set_status(mode, flag, None))
        button_true.clicked.connect(lambda: self.set_status(mode, flag, True))
        button_false.clicked.connect(lambda: self.set_status(mode, flag, False))
        buttons = [button_none, button_true, button_false]
        for index, button in enumerate(buttons):
            button_group.addButton(button)
            button.setCheckable(True)
            layout.addWidget(button, row, col + index)
        self.button[(mode, flag, None)] = button_none
        self.button[(mode, flag, True)] = button_true
        self.button[(mode, flag, False)] = button_false

    def create_all_buttons(self, flag, layout, row, col):
        button_none = QtWidgets.QPushButton("None")
        button_true = QtWidgets.QPushButton("True")
        button_false = QtWidgets.QPushButton("False")
        button_none.clicked.connect(lambda: self.set_all_status(flag, None))
        button_true.clicked.connect(lambda: self.set_all_status(flag, True))
        button_false.clicked.connect(lambda: self.set_all_status(flag, False))
        buttons = [button_none, button_true, button_false]
        for index, button in enumerate(buttons):
            layout.addWidget(button, row, col + index)

    def create_widget(self, modes):
        layout = QtWidgets.QGridLayout()
        layout.setSpacing(0)
        layout.addWidget(QtWidgets.QLabel("Autoware Mode"), 1, 0)
        layout.addWidget(centered_label("  Flags  "), 1, 1)
        layout.addWidget(centered_label("Continuable"), 1, 2, 1, 3)
        layout.addWidget(centered_label("Available"), 1, 5, 1, 3)
        layout.addWidget(centered_label("Stable"), 1, 8, 1, 3)
        self.setLayout(layout)
        self.create_all_buttons(FlagType.Continuable, layout, 0, 2)
        self.create_all_buttons(FlagType.Available, layout, 0, 5)
        self.create_all_buttons(FlagType.Stable, layout, 0, 8)
        for row, (mode, name) in enumerate(modes, start=2):
            layout.addWidget(QtWidgets.QLabel(f"{name} ({mode})"), row, 0)
            self.create_button(FlagType.Continuable, mode, layout, row, 2)
            self.create_button(FlagType.Available, mode, layout, row, 5)
            self.create_button(FlagType.Stable, mode, layout, row, 8)
            label = centered_label("---")
            layout.addWidget(label, row, 1)
            self.flags[mode] = label
