"""Entry point of the mock."""

from typing import Sequence

import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.executors import SingleThreadedExecutor

from .node import MockNode
from .plugin import MockPlugin


def run(
    spec: str = "", default_plugins: Sequence[MockPlugin] = (), node_name: str = "mock", args=None
):
    """Run the mock as a ROS node. Mock packages call this with their default plugins."""
    rclpy.init(args=args)
    node = None
    try:
        node = MockNode(spec=spec, default_plugins=default_plugins, node_name=node_name)
        executor = SingleThreadedExecutor()
        executor.add_node(node)
        executor.spin()
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    finally:
        if node is not None:
            node.destroy_node()
        rclpy.try_shutdown()


def main(args=None):
    run(args=args)


if __name__ == "__main__":
    main()
