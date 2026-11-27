from autoware_adapi_v1_msgs.msg import OperationModeState
from autoware_mock_common import MockPlugin
from autoware_mock_common import service
from autoware_system_msgs.srv import ChangeOperationMode

STATE = "/system/operation_mode/state"

_MODES = {
    ChangeOperationMode.Request.STOP: OperationModeState.STOP,
    ChangeOperationMode.Request.AUTONOMOUS: OperationModeState.AUTONOMOUS,
    ChangeOperationMode.Request.LOCAL: OperationModeState.LOCAL,
    ChangeOperationMode.Request.REMOTE: OperationModeState.REMOTE,
}


class OperationModePlugin(MockPlugin):
    """Default behavior of the operation mode. Requests succeed immediately without transition."""

    publishes = [STATE]

    def on_init(self, ctx):
        self._state = ctx.default_message(STATE)
        self._state.mode = OperationModeState.UNKNOWN
        self._state.is_autoware_control_enabled = True
        self._state.is_in_transition = False
        self._state.is_stop_mode_available = True
        self._state.is_autonomous_mode_available = True
        self._state.is_local_mode_available = False
        self._state.is_remote_mode_available = False
        self._publish(ctx)

    def _publish(self, ctx):
        self._state.stamp = ctx.node.get_clock().now().to_msg()
        ctx.publish(STATE, self._state)

    @service("/system/operation_mode/change_operation_mode")
    def change_operation_mode(self, ctx, req, res):
        mode = _MODES.get(req.mode)
        if mode is None:
            res.status.success = False
            res.status.message = f"unknown mode: {req.mode}"
            return res
        self._state.mode = mode
        self._publish(ctx)
        res.status.success = True
        return res

    @service("/system/operation_mode/change_autoware_control")
    def change_autoware_control(self, ctx, req, res):
        self._state.is_autoware_control_enabled = req.autoware_control
        self._publish(ctx)
        res.status.success = True
        return res
