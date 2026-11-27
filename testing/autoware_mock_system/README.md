# autoware_mock_system

## Overview

This package provides dummy components that mock the Autoware system module for testing.
It allows other components and tools to be tested without launching the actual system nodes.

## Usage

```bash
ros2 run autoware_mock_system system_mock
```

## Mocked interfaces

### Operation mode

| Type      | Name                                             | Message type                                     | Description                                     |
| --------- | ------------------------------------------------ | ------------------------------------------------ | ----------------------------------------------- |
| Publisher | `/system/operation_mode/state`                   | `autoware_adapi_v1_msgs/msg/OperationModeState`  | Current operation mode state (transient local). |
| Service   | `/system/operation_mode/change_operation_mode`   | `autoware_system_msgs/srv/ChangeOperationMode`   | Change the operation mode.                      |
| Service   | `/system/operation_mode/change_autoware_control` | `autoware_system_msgs/srv/ChangeAutowareControl` | Enable or disable autoware control.             |

#### Behavior

- Initial state: `mode = UNKNOWN`, `is_autoware_control_enabled = true`, `is_in_transition = false`.
- Mode availability: `stop` and `autonomous` are available, `local` and `remote` are not.
- `change_operation_mode` switches the mode immediately (no transition) and publishes the new state.
- `change_autoware_control`: TBD

## Parameters

TBD
