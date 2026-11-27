"""Loader of the plugins specified as strings."""

import importlib
import importlib.util
import inspect
import os
from typing import List

from .plugin import MockPlugin


def load_plugins(names: List[str]) -> List[MockPlugin]:
    """
    Load plugins from strings.

    - `module:Class`: Import the module and create the class.
    - `/path/to/file.py:Class`: Load the file and create the class.
    - `/path/to/file.py`: Load the file and create all plugin classes defined in it.
    """
    return [plugin for name in names for plugin in _load_plugin(name)]


def _load_plugin(name: str) -> List[MockPlugin]:
    target, _, class_name = name.rpartition(":") if ":" in name else (name, "", "")
    if target.endswith(".py") or os.path.isfile(target):
        module = _load_file(target)
    else:
        module = importlib.import_module(target)
    if class_name:
        return [getattr(module, class_name)()]
    classes = [
        cls
        for _, cls in inspect.getmembers(module, inspect.isclass)
        if issubclass(cls, MockPlugin) and cls.__module__ == module.__name__
    ]
    if not classes:
        raise ImportError(f"no plugin class found in {name}")
    return [cls() for cls in classes]


def _load_file(path: str):
    module_name = "_mock_plugin_" + os.path.splitext(os.path.basename(path))[0]
    spec = importlib.util.spec_from_file_location(module_name, path)
    if spec is None or spec.loader is None:
        raise ImportError(f"cannot load plugin file: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module
