# autoware_mock_common

## Overview

This package provides the mock engine and the plugin mechanism shared by the mock packages.
See the [README of the testing directory](../README.md) for the design.

## Usage

### Separate process

```bash
ros2 run autoware_mock_common mock --ros-args \
  -p spec:=system \
  -p plugins:="['my_test.plugins:FailModeChange', '/path/to/plugin.py']" \
  -p declarations:="['/path/to/declaration.yaml']"
```

Mock packages (e.g. `autoware_mock_system`) provide their own executables with the default plugins.

### In-process

```python
import rclpy
from autoware_mock_common import MockNode

rclpy.init()
mock = MockNode(spec="system", plugins=[FailModeChange()]).start()  # spin in a background thread
...
records = mock.records("/system/operation_mode/change_operation_mode")
mock.stop()
mock.destroy_node()
```

## Parameters

| Name           | Type         | Description                                                       |
| -------------- | ------------ | ----------------------------------------------------------------- |
| `spec`         | string       | Component name (e.g. `system`) or path of the `*.spec.yaml` file. |
| `plugins`      | string array | User plugins (`module:Class`, `/path/to/file.py[:Class]`).        |
| `declarations` | string array | Paths of the declarative behavior definition files.               |

## Modules

| Module           | Description                                                    |
| ---------------- | -------------------------------------------------------------- |
| `spec.py`        | Loader of the external specification files.                    |
| `plugin.py`      | `MockPlugin` base class and `@service` / `@subscription`.      |
| `declaration.py` | Declarative behavior definition (used as a plugin internally). |
| `loader.py`      | Loader of the plugins specified as strings.                    |
| `node.py`        | `MockNode` (mock engine), `MockContext`, and recording.        |
| `main.py`        | Entry point of the mock.                                       |
