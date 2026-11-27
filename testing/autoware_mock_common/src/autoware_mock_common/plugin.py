"""Plugin interface of the mock."""

import inspect
from typing import Callable
from typing import Dict
from typing import List


def service(name: str):
    """Mark a method as the handler of the service server: method(ctx, req, res) -> res."""

    def decorator(func):
        func._mock_service = name
        return func

    return decorator


def subscription(name: str):
    """Mark a method as the callback of the subscriber: method(ctx, msg)."""

    def decorator(func):
        func._mock_subscription = name
        return func

    return decorator


class MockPlugin:
    """
    Base class of the mock plugins.

    Plugins declare the interfaces they own in advance so that conflicts can be detected at startup.
    Services are declared by the `@service` decorator.
    Publishers and diagnostics are declared by the `publishes` and `diagnostics` class attributes.
    Callbacks can be normal functions or coroutines.
    """

    publishes: List[str] = []
    diagnostics: List[str] = []

    @property
    def plugin_name(self) -> str:
        return type(self).__name__

    def on_init(self, ctx) -> None:
        """Call when the mock starts."""

    def get_services(self) -> Dict[str, Callable]:
        return {method._mock_service: method for method in self._marked("_mock_service")}

    def get_subscriptions(self) -> Dict[str, List[Callable]]:
        result: Dict[str, List[Callable]] = {}
        for method in self._marked("_mock_subscription"):
            result.setdefault(method._mock_subscription, []).append(method)
        return result

    def get_publishes(self) -> List[str]:
        return list(self.publishes)

    def get_diagnostics(self) -> List[str]:
        return list(self.diagnostics)

    def _marked(self, attr: str) -> List[Callable]:
        methods = []
        for name, _ in inspect.getmembers(type(self), callable):
            method = getattr(self, name)
            if hasattr(method, attr):
                methods.append(method)
        return methods
