import os
import time

from autoware_mock_common import MockError
from autoware_mock_common import MockNode
from autoware_mock_common import MockPlugin
from autoware_mock_common import service
from autoware_mock_common import subscription
from diagnostic_msgs.msg import DiagnosticArray
from diagnostic_msgs.msg import DiagnosticStatus
import pytest
import rclpy
from rclpy.executors import SingleThreadedExecutor
from rclpy.qos import DurabilityPolicy
from rclpy.qos import QoSProfile
from std_msgs.msg import String
from std_srvs.srv import Trigger

SPEC = os.path.join(os.path.dirname(__file__), "specs", "test.spec.yaml")


class FailPlugin(MockPlugin):
    @service("/test/trigger")
    def trigger(self, ctx, req, res):
        res.success = False
        res.message = "fail"
        return res


class StatePlugin(MockPlugin):
    publishes = ["/test/state"]

    def on_init(self, ctx):
        ctx.publish("/test/state", String(data="init"))

    @subscription("/test/input")
    def on_input(self, ctx, msg):
        ctx.publish("/test/state", String(data=msg.data))


@pytest.fixture
def ros():
    rclpy.init()
    client = rclpy.create_node("test_client")
    executor = SingleThreadedExecutor()
    executor.add_node(client)
    mocks = []

    def create(**kwargs):
        mock = MockNode(spec=SPEC, **kwargs).start()
        mocks.append(mock)
        return mock

    yield client, executor, create
    for mock in mocks:
        mock.stop()
        mock.destroy_node()
    client.destroy_node()
    rclpy.shutdown()


def call(client, executor, timeout=5.0):
    cli = client.create_client(Trigger, "/test/trigger")
    assert cli.wait_for_service(timeout_sec=timeout)
    future = cli.call_async(Trigger.Request())
    executor.spin_until_future_complete(future, timeout_sec=timeout)
    return future.result()


def spin_until(executor, condition, timeout=5.0):
    end = time.monotonic() + timeout
    while not condition() and time.monotonic() < end:
        executor.spin_once(timeout_sec=0.05)
    return condition()


def test_builtin_service(ros):
    client, executor, create = ros
    mock = create()
    assert call(client, executor).success
    kinds = [r.kind for r in mock.records("/test/trigger")]
    assert kinds == ["request", "response"]


def test_plugin_overrides_declaration(ros):
    client, executor, create = ros
    declaration = {
        "services": {"/test/trigger": {"response": {"success": True, "message": "decl"}}}
    }
    create(plugins=[FailPlugin()], declarations=[declaration])
    res = call(client, executor)
    assert not res.success and res.message == "fail"


def test_declaration_delay(ros):
    client, executor, create = ros
    declaration = {"services": {"/test/trigger": {"delay": 0.5, "response": {"message": "late"}}}}
    create(declarations=[declaration])
    start = time.monotonic()
    res = call(client, executor)
    assert res.message == "late"
    assert time.monotonic() - start >= 0.5


def test_conflict_in_same_layer(ros):
    _, _, create = ros
    with pytest.raises(MockError):
        create(plugins=[FailPlugin(), FailPlugin()])


def test_publish_and_subscription(ros):
    client, executor, create = ros
    mock = create(plugins=[StatePlugin()])
    received = []
    qos = QoSProfile(depth=1, durability=DurabilityPolicy.TRANSIENT_LOCAL)
    client.create_subscription(String, "/test/state", lambda m: received.append(m.data), qos)
    pub = client.create_publisher(String, "/test/input", 1)
    assert spin_until(executor, lambda: "init" in received)
    assert spin_until(executor, lambda: pub.get_subscription_count() > 0)
    pub.publish(String(data="hello"))
    assert spin_until(executor, lambda: "hello" in received)
    assert [r.kind for r in mock.records("/test/input")] == ["received"]


def test_undeclared_publish(ros):
    _, _, create = ros

    class Undeclared(MockPlugin):
        def on_init(self, ctx):
            ctx.publish("/test/state", String())

    with pytest.raises(MockError):
        create(plugins=[Undeclared()])


def test_diagnostics(ros):
    client, executor, create = ros
    declaration = {"diagnostics": {"test_node: status": {"level": "ERROR", "message": "error"}}}
    create(declarations=[declaration])
    received = []
    client.create_subscription(DiagnosticArray, "/diagnostics", received.append, 10)
    assert spin_until(executor, lambda: len(received) >= 2)
    status = received[-1].status[0]
    assert status.name == "test_node: status"
    assert status.level == DiagnosticStatus.ERROR
