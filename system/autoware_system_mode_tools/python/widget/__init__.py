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


from python_qt_binding import QtWidgets

from .driving_mode import DrivingModeControl
from .gate_status import CommandGateDisplay
from .gate_status import TrajectoryGateDisplay
from .operation_mode import OperationModeControl


class MainWidget(QtWidgets.QWidget):
    def __init__(self, node):
        super().__init__()
        modes = [
            (101, "Stop"),
            (102, "Autonomous"),
        ]
        self.operation_mode_control = OperationModeControl(node, modes)
        self.modes_control = DrivingModeControl(node, modes)
        self.status_trajectory = TrajectoryGateDisplay(node)
        self.status_command = CommandGateDisplay(node)

        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.operation_mode_control)
        layout.addWidget(self.modes_control)
        layout.addWidget(self.status_trajectory)
        layout.addWidget(self.status_command)
        self.setLayout(layout)
