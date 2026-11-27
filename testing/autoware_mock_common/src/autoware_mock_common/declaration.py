"""Declarative behavior definition."""

import os
from typing import Callable
from typing import Dict
from typing import List
from typing import Union

from rclpy.task import Future
from rosidl_runtime_py import set_message_fields
import yaml

from .plugin import MockPlugin


class Declaration(MockPlugin):
    """
    Behavior defined declaratively, used as a plugin.

    services:
      <name>:
        delay: <seconds>
        response: <fields of the response>
    diagnostics:
      <name>:
        level: OK | WARN | ERROR | STALE
        message: <message>
    """

    def __init__(self, data: dict, name: str = "declaration"):
        self._name = name
        self._services = data.get("services") or {}
        self._diagnostics = data.get("diagnostics") or {}

    @classmethod
    def load(cls, source: Union[dict, str]) -> "Declaration":
        if isinstance(source, dict):
            return cls(source)
        with open(source) as fp:
            return cls(yaml.safe_load(fp) or {}, os.path.basename(source))

    @property
    def plugin_name(self) -> str:
        return self._name

    def on_init(self, ctx) -> None:
        for name, item in self._diagnostics.items():
            ctx.set_diag(name, item.get("level", "OK"), item.get("message", ""))

    def get_services(self) -> Dict[str, Callable]:
        return {name: self._service_handler(item or {}) for name, item in self._services.items()}

    def get_subscriptions(self) -> Dict[str, List[Callable]]:
        return {}

    def get_publishes(self) -> List[str]:
        return []

    def get_diagnostics(self) -> List[str]:
        return list(self._diagnostics)

    @staticmethod
    def _service_handler(item: dict) -> Callable:
        delay = item.get("delay", 0.0)
        response = item.get("response") or {}

        async def handler(ctx, req, res):
            if delay > 0:
                await _sleep(ctx.node, delay)
            set_message_fields(res, response)
            return res

        return handler


async def _sleep(node, seconds: float) -> None:
    future = Future(executor=node.executor)
    timer = None

    def callback():
        timer.cancel()
        node.destroy_timer(timer)
        future.set_result(None)

    timer = node.create_timer(seconds, callback, callback_group=node._mock_group)
    await future
