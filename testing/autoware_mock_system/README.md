# autoware_mock_system

## Overview

This package provides dummy components that mock the Autoware system module for testing.
It allows other components and tools to be tested without launching the actual system nodes.

## Usage

```bash
ros2 run autoware_mock_system system_mock
```

## In-process usage

```python
from autoware_mock_system import create_mock

mock = create_mock(plugins=[...]).start()
```

## Interfaces

The interfaces are defined in `autoware_mock_specs/specs/system.spec.yaml`.

## Default plugins

### OperationModePlugin

- Initial state: `mode = UNKNOWN`, `is_autoware_control_enabled = true`, `is_in_transition = false`.
- Mode availability: `stop` and `autonomous` are available, `local` and `remote` are not.
- `change_operation_mode` switches the mode immediately (no transition) and publishes the new state.
  An unknown mode returns failure.
- `change_autoware_control` switches `is_autoware_control_enabled` immediately and publishes the new state.
