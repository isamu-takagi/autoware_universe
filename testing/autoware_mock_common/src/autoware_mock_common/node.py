"""Mock engine that creates the interfaces from the external specification."""

from dataclasses import dataclass
import inspect
import threading
from typing import Any
from typing import Dict
from typing import List
from typing import Optional
from typing import Sequence
from typing import Union

from diagnostic_msgs.msg import DiagnosticArray
from diagnostic_msgs.msg import DiagnosticStatus
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.executors import SingleThreadedExecutor
from rclpy.node import Node
from rclpy.parameter import Parameter
from rclpy.time import Time

from .declaration import Declaration
from .loader import load_plugins
from .plugin import MockPlugin
from .spec import Spec
from .spec import load_spec

# Layers of the behavior definitions, in order of priority.
LAYER_NAMES = ("user plugin", "user declaration", "default plugin")

_DIAG_LEVELS = {
    "OK": DiagnosticStatus.OK,
    "WARN": DiagnosticStatus.WARN,
    "ERROR": DiagnosticStatus.ERROR,
    "STALE": DiagnosticStatus.STALE,
}


class MockError(Exception):
    """Error in the mock configuration or usage."""


@dataclass
class Record:
    """Interaction of the mock with other components."""

    stamp: Time
    kind: str  # request, response, received, published
    data: Any


class MockContext:
    """Functions provided to a plugin."""

    def __init__(self, mock: "MockNode", plugin: MockPlugin):
        self._mock = mock
        self._plugin = plugin

    @property
    def node(self) -> Node:
        return self._mock

    @property
    def spec(self) -> Spec:
        return self._mock.spec

    @property
    def shared(self) -> Dict[str, Any]:
        return self._mock.shared

    def default_message(self, name: str):
        return self._mock.spec.interfaces[name].msg_type()

    def publish(self, name: str, msg) -> None:
        self._mock._publish(self._plugin, name, msg)

    def set_diag(self, name: str, level: Union[str, bytes], message: str = "") -> None:
        self._mock._set_diag(self._plugin, name, level, message)

    def client(self, name: str):
        return self._mock._mock_clients[name]


class MockNode(Node):
    """
    Mock of a component created from the external specification.

    The behavior of each interface is determined by the layers in `LAYER_NAMES`,
    and the built-in default behavior is used if no layer defines it.
    """

    def __init__(
        self,
        spec: Optional[str] = None,
        plugins: Sequence[Union[MockPlugin, str]] = (),
        declarations: Sequence[Union[dict, str]] = (),
        default_plugins: Sequence[MockPlugin] = (),
        node_name: str = "mock",
        **kwargs,
    ):
        super().__init__(node_name, **kwargs)
        spec = self.declare_parameter("spec", spec or "").value
        plugins = list(plugins) + self._string_array("plugins")
        declarations = list(declarations) + self._string_array("declarations")
        if not spec:
            raise MockError("spec is not specified")

        self.spec = load_spec(spec)
        self.shared: Dict[str, Any] = {}
        self._mock_group = ReentrantCallbackGroup()
        self._records_lock = threading.Lock()
        self._records: Dict[str, List[Record]] = {}
        self._layers: List[List[MockPlugin]] = [
            load_plugins([p for p in plugins if isinstance(p, str)])
            + [p for p in plugins if not isinstance(p, str)],
            [Declaration.load(d) for d in declarations],
            list(default_plugins),
        ]
        self._resolve_owners()
        self._create_interfaces()

        # Initialize from the lower layer so that the upper layer can overwrite the shared state.
        for layer in reversed(self._layers):
            for plugin in layer:
                plugin.on_init(self._contexts[id(plugin)])

        self._mock_executor = None
        self._mock_thread = None

    # ------------------------------------------------------------------
    # Public API for tests
    # ------------------------------------------------------------------

    def records(self, name: Optional[str] = None) -> List[Record]:
        """Return the records of the interface (all records if name is None)."""
        with self._records_lock:
            if name is not None:
                return list(self._records.get(name, []))
            return [r for records in self._records.values() for r in records]

    def clear_records(self) -> None:
        with self._records_lock:
            self._records.clear()

    def start(self) -> "MockNode":
        """Spin the mock in a background thread (for in-process use)."""
        self._mock_executor = SingleThreadedExecutor(context=self.context)
        self._mock_executor.add_node(self)
        self._mock_thread = threading.Thread(target=self._mock_executor.spin, daemon=True)
        self._mock_thread.start()
        return self

    def stop(self) -> None:
        if self._mock_executor is not None:
            self._mock_executor.shutdown()
            self._mock_thread.join()
            self._mock_executor = None
            self._mock_thread = None

    # ------------------------------------------------------------------
    # Setup
    # ------------------------------------------------------------------

    def _string_array(self, name: str) -> List[str]:
        value = self.declare_parameter(name, Parameter.Type.STRING_ARRAY).value
        return [v for v in (value or []) if v]

    def _plugins(self):
        for index, layer in enumerate(self._layers):
            for plugin in layer:
                yield index, plugin

    def _resolve_owners(self) -> None:
        self._contexts = {id(p): MockContext(self, p) for _, p in self._plugins()}
        self._service_owners = self._resolve("service", lambda p: p.get_services().keys())
        self._publish_owners = self._resolve("publisher", lambda p: p.get_publishes())
        self._diag_owners = self._resolve("diagnostics", lambda p: p.get_diagnostics())

        for _, plugin in self._plugins():
            for name in plugin.get_services():
                self.spec.get(name, "service", "server")
            for name in plugin.get_publishes():
                self.spec.get(name, "topic", "publisher")
            for name in plugin.get_subscriptions():
                self.spec.get(name, "topic", "subscriber")
            for name in plugin.get_diagnostics():
                if name not in self.spec.diagnostics:
                    raise MockError(f"diagnostics '{name}' is not defined in {self.spec.path}")

    def _resolve(self, label: str, getter) -> Dict[str, MockPlugin]:
        """Find the owner of each interface. Conflicts in the same layer are errors."""
        owners: Dict[str, MockPlugin] = {}
        for index, layer in enumerate(self._layers):
            layer_owners: Dict[str, MockPlugin] = {}
            for plugin in layer:
                for name in getter(plugin):
                    other = layer_owners.get(name)
                    if other is not None:
                        raise MockError(
                            f"{label} '{name}' is defined by both {other.plugin_name} and "
                            f"{plugin.plugin_name} in the {LAYER_NAMES[index]} layer"
                        )
                    layer_owners[name] = plugin
            for name, plugin in layer_owners.items():
                owners.setdefault(name, plugin)
        return owners

    def _create_interfaces(self) -> None:
        self._mock_publishers = {}
        self._mock_clients = {}
        self._mock_services = {}
        self._mock_subscriptions = {}
        for interface in self.spec.interfaces.values():
            name = interface.name
            if interface.role == "publisher":
                self._mock_publishers[name] = self.create_publisher(
                    interface.msg_type, name, interface.qos, callback_group=self._mock_group
                )
            elif interface.role == "subscriber":
                self._mock_subscriptions[name] = self.create_subscription(
                    interface.msg_type,
                    name,
                    self._subscription_callback(name),
                    interface.qos,
                    callback_group=self._mock_group,
                )
            elif interface.role == "server":
                self._mock_services[name] = self.create_service(
                    interface.msg_type,
                    name,
                    self._service_callback(name),
                    callback_group=self._mock_group,
                )
            elif interface.role == "client":
                self._mock_clients[name] = self.create_client(
                    interface.msg_type, name, callback_group=self._mock_group
                )

        self._diag_states = {}
        self._diag_timers = []
        for diag in self.spec.diagnostics.values():
            self._diag_states[diag.name] = (DiagnosticStatus.OK, "")
            self._diag_timers.append(
                self.create_timer(
                    1.0 / diag.rate["min"],
                    self._diag_callback(diag.name),
                    callback_group=self._mock_group,
                )
            )

    # ------------------------------------------------------------------
    # Callbacks
    # ------------------------------------------------------------------

    def _service_callback(self, name: str):
        owner = self._service_owners.get(name)
        handler = owner.get_services()[name] if owner else None
        context = self._contexts[id(owner)] if owner else None

        async def callback(req, res):
            self._record(name, "request", req)
            if handler is None:
                result = _builtin_service(req, res)
            else:
                result = handler(context, req, res)
                if inspect.isawaitable(result):
                    result = await result
                if result is None:
                    result = res
            self._record(name, "response", result)
            return result

        return callback

    def _subscription_callback(self, name: str):
        handlers = []
        for _, plugin in self._plugins():
            for handler in plugin.get_subscriptions().get(name, []):
                handlers.append((handler, self._contexts[id(plugin)]))

        async def callback(msg):
            self._record(name, "received", msg)
            for handler, context in handlers:
                result = handler(context, msg)
                if inspect.isawaitable(result):
                    await result

        return callback

    def _diag_callback(self, name: str):
        def callback():
            level, message = self._diag_states[name]
            msg = DiagnosticArray()
            msg.header.stamp = self.get_clock().now().to_msg()
            msg.status.append(DiagnosticStatus(level=level, name=name, message=message))
            self._mock_publishers["/diagnostics"].publish(msg)
            self._record(name, "published", msg.status[0])

        return callback

    # ------------------------------------------------------------------
    # Functions for the context
    # ------------------------------------------------------------------

    def _publish(self, plugin: MockPlugin, name: str, msg) -> None:
        if name not in plugin.get_publishes():
            raise MockError(f"{plugin.plugin_name} publishes '{name}' without declaring it")
        if self._publish_owners[name] is not plugin:
            return  # Overridden by the upper layer.
        self._mock_publishers[name].publish(msg)
        self._record(name, "published", msg)

    def _set_diag(self, plugin: MockPlugin, name: str, level, message: str) -> None:
        if name not in plugin.get_diagnostics():
            raise MockError(f"{plugin.plugin_name} sets diagnostics '{name}' without declaring it")
        if self._diag_owners[name] is not plugin:
            return  # Overridden by the upper layer.
        if isinstance(level, str):
            level = _DIAG_LEVELS[level.upper()]
        self._diag_states[name] = (level, message)

    def _record(self, name: str, kind: str, data) -> None:
        record = Record(self.get_clock().now(), kind, data)
        with self._records_lock:
            self._records.setdefault(name, []).append(record)


def _builtin_service(req, res):
    """Return success immediately."""
    status = getattr(res, "status", None)
    if status is not None and hasattr(status, "success"):
        status.success = True
    elif hasattr(res, "success"):
        res.success = True
    return res
