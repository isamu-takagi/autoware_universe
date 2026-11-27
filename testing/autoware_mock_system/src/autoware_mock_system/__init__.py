from autoware_mock_common import MockNode

from .plugins.operation_mode import OperationModePlugin

SPEC = "system"
NODE_NAME = "system_mock"


def default_plugins():
    return [OperationModePlugin()]


def create_mock(**kwargs) -> MockNode:
    """Create the system mock for in-process use."""
    kwargs.setdefault("node_name", NODE_NAME)
    return MockNode(spec=SPEC, default_plugins=default_plugins(), **kwargs)
