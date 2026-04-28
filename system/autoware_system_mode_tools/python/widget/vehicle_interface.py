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


from autoware_system_mode_tools.utils import default_qos
from autoware_vehicle_msgs.msg import ControlModeReport
from autoware_vehicle_msgs.srv import ControlModeCommand
from python_qt_binding import QtCore
from python_qt_binding import QtWidgets


class VehicleInterfaceControl(QtWidgets.QWidget):
    def __init__(self, node):
        super().__init__()
        self.mode = None
        self.pub = node.create_publisher(
            ControlModeReport, "/vehicle/status/control_mode", default_qos(1)
        )
        self.srv = node.create_service(
            ControlModeCommand, "/control/control_mode_request", self.on_request
        )
        self.clock = node.get_clock()
        self.timer = node.create_timer(1.0, self.on_timer)
        self.create_widget()

    def create_widget(self):
        modes = [
            (ControlModeCommand.Request.AUTONOMOUS, "Autoware"),
            (ControlModeCommand.Request.AUTONOMOUS_STEER_ONLY, "Steering"),
            (ControlModeCommand.Request.AUTONOMOUS_VELOCITY_ONLY, "Velocity"),
            (ControlModeCommand.Request.MANUAL, "Manual"),
        ]
        self.status = QtWidgets.QLabel("None")
        self.status.setAlignment(QtCore.Qt.AlignCenter)
        self.status.setStyleSheet("border: 1px solid black;")

        layout = QtWidgets.QGridLayout()
        layout.setSpacing(0)
        layout.addWidget(QtWidgets.QLabel("Control Mode"), 0, 0)
        layout.addWidget(self.status, 0, 1)
        for col, (mode, name) in enumerate(modes, start=2):
            button = QtWidgets.QPushButton(name)
            button.clicked.connect(lambda _, mode=mode: self.change_mode(mode))
            layout.addWidget(button, 0, col)
        self.setLayout(layout)

    def on_timer(self):
        self.publish()

    def publish(self):
        if self.mode is not None:
            msg = ControlModeReport()
            msg.stamp = self.clock.now().to_msg()
            msg.mode = self.mode
            self.pub.publish(msg)

    def change_mode(self, mode):
        self.mode = mode
        self.status.setText(self.__mode_text.get(mode, "Unknown"))
        self.publish()

    def on_request(self, request, response):
        mode = self.__request_to_report.get(request.mode)
        if mode is None:
            response.success = False
            return response
        else:
            self.change_mode(mode)
            response.success = True
            return response

    __request_to_report = {
        ControlModeCommand.Request.NO_COMMAND: ControlModeReport.NO_COMMAND,
        ControlModeCommand.Request.AUTONOMOUS: ControlModeReport.AUTONOMOUS,
        ControlModeCommand.Request.AUTONOMOUS_STEER_ONLY: ControlModeReport.AUTONOMOUS_STEER_ONLY,
        ControlModeCommand.Request.AUTONOMOUS_VELOCITY_ONLY: ControlModeReport.AUTONOMOUS_VELOCITY_ONLY,
        ControlModeCommand.Request.MANUAL: ControlModeReport.MANUAL,
    }

    __mode_text = {
        ControlModeReport.NO_COMMAND: "No Command",
        ControlModeReport.AUTONOMOUS: "Autoware",
        ControlModeReport.AUTONOMOUS_STEER_ONLY: "Steering",
        ControlModeReport.AUTONOMOUS_VELOCITY_ONLY: "Velocity",
        ControlModeReport.MANUAL: "Manual",
        ControlModeReport.DISENGAGED: "Disengaged",
        ControlModeReport.NOT_READY: "Not Ready",
    }
