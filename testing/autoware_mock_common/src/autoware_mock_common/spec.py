"""Loader of the external specification files (*.spec.yaml)."""

from dataclasses import dataclass
from dataclasses import field
import os
from typing import Any
from typing import Dict
from typing import Optional

from ament_index_python.packages import get_package_share_directory
from rclpy.qos import DurabilityPolicy
from rclpy.qos import QoSProfile
from rclpy.qos import ReliabilityPolicy
from rosidl_runtime_py.utilities import get_message
from rosidl_runtime_py.utilities import get_service
import yaml

SPEC_PACKAGE = "autoware_mock_specs"
SPEC_VERSION = 1

_ROLES = {"topic": ("publisher", "subscriber"), "service": ("server", "client")}
_RELIABILITY = {
    "reliable": ReliabilityPolicy.RELIABLE,
    "best_effort": ReliabilityPolicy.BEST_EFFORT,
}
_DURABILITY = {
    "volatile": DurabilityPolicy.VOLATILE,
    "transient_local": DurabilityPolicy.TRANSIENT_LOCAL,
}


class SpecError(Exception):
    """Error in the external specification file."""


@dataclass
class Interface:
    """Topic or service defined in the external specification."""

    name: str
    kind: str
    role: str
    type_name: str
    msg_type: Any
    qos: Optional[QoSProfile] = None
    rate: Dict[str, Optional[float]] = field(default_factory=dict)
    response_time: Dict[str, Optional[float]] = field(default_factory=dict)
    description: str = ""


@dataclass
class Diagnostic:
    """Diagnostic status defined in the external specification."""

    name: str
    rate: Dict[str, Optional[float]]
    description: str = ""


@dataclass
class Spec:
    """External specification of a component."""

    component: str
    path: str
    interfaces: Dict[str, Interface]
    diagnostics: Dict[str, Diagnostic]

    def get(self, name: str, kind: str, role: str) -> Interface:
        interface = self.interfaces.get(name)
        if interface is None or interface.kind != kind or interface.role != role:
            raise SpecError(f"{kind} {role} '{name}' is not defined in {self.path}")
        return interface


def find_spec_file(name: str) -> str:
    """Return the path of the spec file from a component name or a file path."""
    if os.path.isfile(name):
        return name
    share = get_package_share_directory(SPEC_PACKAGE)
    path = os.path.join(share, "specs", f"{name}.spec.yaml")
    if not os.path.isfile(path):
        raise SpecError(f"spec file not found: {name}")
    return path


def load_spec(name: str) -> Spec:
    """Load the spec file from a component name (e.g. 'system') or a file path."""
    path = find_spec_file(name)
    with open(path) as fp:
        data = yaml.safe_load(fp)
    try:
        return _parse_spec(data, path)
    except (KeyError, TypeError, ValueError) as error:
        raise SpecError(f"invalid spec file {path}: {error!r}") from error


def _parse_spec(data: dict, path: str) -> Spec:
    if data.get("version") != SPEC_VERSION:
        raise SpecError(f"unsupported version in {path}: {data.get('version')}")
    interfaces = {}
    for item in data.get("interfaces") or []:
        interface = _parse_interface(item)
        if interface.name in interfaces:
            raise SpecError(f"duplicated interface in {path}: {interface.name}")
        interfaces[interface.name] = interface
    diagnostics = {}
    for item in data.get("diagnostics") or []:
        diag = Diagnostic(item["name"], item["rate"], item.get("description", ""))
        if not diag.rate.get("min"):
            raise SpecError(f"rate.min is required for diagnostics in {path}: {diag.name}")
        diagnostics[diag.name] = diag
    if diagnostics and "/diagnostics" not in interfaces:
        raise SpecError(f"/diagnostics publisher is required for diagnostics in {path}")
    return Spec(data["component"], path, interfaces, diagnostics)


def _parse_interface(item: dict) -> Interface:
    kind = item["kind"]
    role = item["role"]
    if role not in _ROLES.get(kind, ()):
        raise ValueError(f"invalid kind/role: {kind}/{role}")
    type_name = item["type"]
    if kind == "topic":
        msg_type = get_message(type_name)
        qos = _parse_qos(item["qos"])
    else:
        msg_type = get_service(type_name)
        qos = None
    return Interface(
        name=item["name"],
        kind=kind,
        role=role,
        type_name=type_name,
        msg_type=msg_type,
        qos=qos,
        rate=item.get("rate") or {},
        response_time=item.get("response_time") or {},
        description=item.get("description", ""),
    )


def _parse_qos(data: dict) -> QoSProfile:
    return QoSProfile(
        depth=data.get("depth", 1),
        reliability=_RELIABILITY[data["reliability"]],
        durability=_DURABILITY[data["durability"]],
    )
