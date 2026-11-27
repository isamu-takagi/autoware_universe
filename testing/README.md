# Autoware mock components

## Overview

This directory provides dummy components that mock Autoware modules for testing.

## Purpose

- **Clarify interfaces and their behavior**:
  Each mock is a drop-in replacement for the actual component.
  Implementing it makes the interface of the component and its expected behavior explicit.
- **Component-level verification**:
  By replacing surrounding components with mocks,
  each component can be verified in isolation without being affected by changes in other components.

## Design

### Compatibility with the actual component

Each item of a component is classified as either an external specification or an implementation detail.

- **External specification**:
  Items that other components or tools depend on.
  These must be explicitly documented, and the mock must match the actual component.
- **Implementation detail**:
  Items that are not part of the external specification.
  The mock may differ from the actual component.

The default classification is as follows.

| Item                                              | Classification                                               |
| ------------------------------------------------- | ------------------------------------------------------------ |
| Topic / service name and type                     | External specification                                       |
| QoS reliability and durability                    | External specification                                       |
| QoS depth                                         | Implementation detail (a recommended value is given)         |
| Meaning of messages and state transition behavior | External specification                                       |
| Topic publish rate and service response time      | External specification if there is a requirement             |
| Node name and node composition                    | Implementation detail, unless referenced by other components |
| Parameters and launch arguments                   | Implementation detail, unless configured by other components |
| Diagnostic status name                            | External specification, even if it contains the node name    |
| Diagnostic status publish rate                    | External specification                                       |
| Diagnostic status level, hardware ID, and values  | Implementation detail for now                                |

### External specification format

The external specification of each component is described in a machine-readable format such as YAML,
so that it can be parsed by tools (e.g. to generate mocks or check compatibility).

#### Schema (draft)

One file per component, named `<component>.spec.yaml` (e.g. `system.spec.yaml`).

```yaml
version: 1 # schema version
component: system
interfaces:
  - name: /system/operation_mode/state
    kind: topic # topic | service
    role: publisher # topic: publisher | subscriber, service: server | client
    type: autoware_adapi_v1_msgs/msg/OperationModeState
    qos:
      depth: 1 # recommended value
      reliability: reliable # reliable | best_effort
      durability: transient_local # volatile | transient_local
    rate: # optional, only if there is a requirement [Hz]
      min: null
      max: null
    description: Current operation mode state.

  - name: /system/operation_mode/change_operation_mode
    kind: service
    role: server
    type: autoware_system_msgs/srv/ChangeOperationMode
    response_time: # optional, only if there is a requirement [s]
      max: null
    description: Change the operation mode.

  - name: /system/operation_mode/change_autoware_control
    kind: service
    role: server
    type: autoware_system_msgs/srv/ChangeAutowareControl
    description: Enable or disable autoware control.

  - name: /diagnostics
    kind: topic
    role: publisher
    type: diagnostic_msgs/msg/DiagnosticArray
    qos:
      depth: 1 # recommended value, same as diagnostic_updater
      reliability: reliable
      durability: volatile

diagnostics:
  - name: "operation_mode_transition_manager: example_status" # placeholder
    rate: # [Hz]
      min: 1.0
      max: null
    description: Example diagnostic status.
```

| Field                        | Required | Description                                               |
| ---------------------------- | -------- | --------------------------------------------------------- |
| `version`                    | Yes      | Schema version of the file.                               |
| `component`                  | Yes      | Component name.                                           |
| `interfaces[].name`          | Yes      | Topic or service name.                                    |
| `interfaces[].kind`          | Yes      | `topic` or `service`.                                     |
| `interfaces[].role`          | Yes      | Role from the viewpoint of the component.                 |
| `interfaces[].type`          | Yes      | Message or service type.                                  |
| `interfaces[].qos`           | Topic    | QoS settings. Required for topics.                        |
| `interfaces[].qos.depth`     | No       | Recommended history depth.                                |
| `interfaces[].rate`          | No       | Publish rate requirement (publisher only).                |
| `interfaces[].response_time` | No       | Response time requirement (server only).                  |
| `interfaces[].description`   | No       | Meaning of the interface.                                 |
| `diagnostics[].name`         | Yes      | Name of the diagnostic status published by the component. |
| `diagnostics[].rate`         | Yes      | Publish rate of the diagnostic status.                    |
| `diagnostics[].description`  | No       | Meaning of the diagnostic status.                         |

Notes:

- The role is described from the viewpoint of the component, so both inputs and outputs are listed.
- Behavior (e.g. state transitions) is written in `description` for now.
  It is planned to be formalized in the future, possibly in a common format with the mock behavior definition.
- Actions are not supported because they are not currently needed.
- The version of the interfaces themselves is not managed for now.
- The C++ consistency test matches each header struct to an interface by `name`.
- QoS depth only affects the history kept by each node and does not affect compatibility,
  so it is an implementation detail. The value in the specification is a recommendation.
  Consistency tests do not fail on a depth mismatch, but output a warning.
- Diagnostics are described in a separate `diagnostics` section, because the `/diagnostics` topic is shared
  by all components and the name of each status, not the topic, is referenced by other components
  (e.g. `diagnostic_graph_aggregator`).
  The `/diagnostics` topic itself is listed in `interfaces` as a normal topic.
  Its QoS follows `diagnostic_updater` (reliable, volatile, recommended depth 1).
- Only the status name and publish rate are the external specification for now.
  Level conditions, hardware ID, and values may be added in the future.
- The publish rate of a diagnostic status is always required,
  because consumers such as `diagnostic_graph_aggregator` judge timeout based on it.

### Relationship with `autoware_component_interface_specs`

`autoware_component_interface_specs` (and `autoware_component_interface_specs_universe`)
already defines topic/service names, types, and QoS as C++ headers, which overlaps with the external specification.

- **Current**: The external specification files are maintained separately,
  and tests check that they are consistent with the C++ headers.
- **Future**: Consider making the external specification files the single source of truth
  and generating the C++ headers from them.

### Relationship with `autoware_system_designer`

`autoware_system_designer` (`src/core/autoware_system_designer`) also defines its own YAML formats
that describe interfaces, such as the inputs/outputs of nodes (`*.node.yaml`)
and the external interfaces of modules (`*.module.yaml`), including message types and QoS.

- **Current**: The mocks use their own dedicated format (`*.spec.yaml`) as a test-only feature.
- **Future**: When the external specifications are integrated into the spec package and formalized,
  the format should be aligned with `autoware_system_designer`
  (e.g. adopting its format, generating one from the other, or sharing a common schema).

### Package structure

The packages are divided by role.

| Package                    | Role                                                 | Status    |
| -------------------------- | ---------------------------------------------------- | --------- |
| `autoware_mock_specs`      | External specification files of all components       | Prototype |
| `autoware_mock_specs_test` | Tests for consistency of the external specifications | Planned   |
| `autoware_mock_common`     | Mock engine and plugin mechanism                     | Prototype |
| `autoware_mock_system`     | Mock of the system component                         | Prototype |

- **Specification package**:
  The external specifications of all components are collected in a single package,
  independent of the mock implementations.
- **Test package**:
  Consistency tests are provided as a dedicated package, separated from the specifications and the mocks.
  - Consistency between the external specification files and `autoware_component_interface_specs` (C++ test).
  - Consistency between the external specification files and the mocks (runtime test using the ROS graph).
- **Common package**:
  Provides the spec-driven mock engine and the plugin mechanism shared by all mock packages.
- **Mock packages**:
  Each mocked component is provided as a separate package.
  A mock replaces the entire component (e.g. system, planning), not individual nodes within it.
  Each mock package provides the default plugin of the component (e.g. operation mode state transitions).

The specification packages are named with `mock` because they are currently test-only features.
When the external specifications become the single source of truth,
they will be integrated into the spec package (e.g. `autoware_component_interface_specs`).

### Mock engine

The mock engine in `autoware_mock_common` reads a `*.spec.yaml` file
and automatically creates all interfaces (publishers, subscribers, services, and diagnostics) defined in it.
This guarantees that the names, types, and QoS of the mock always match the external specification.
Interfaces not defined in the specification cannot be created.

### Behavior

The behavior of each interface is determined by the following layers, in order of priority.
If a layer does not define the behavior of an interface, the next layer is used.

1. **User plugins**: Python plugins loaded by tests.
2. **User declarations**: Declarative behavior definitions loaded by tests.
3. **Default plugin**: Provided by each mock package (e.g. operation mode state transitions).
4. **Built-in default**: Provided by the mock engine.

If multiple definitions conflict on the same interface:

- **Same layer**: It is an error (e.g. two user plugins handle the same service), and the mock fails to start.
- **Different layers**: The upper layer wins (e.g. a user plugin overrides a user declaration).

Whether definitions conflict depends on the interface type.

| Interface   | Conflict                                                             |
| ----------- | -------------------------------------------------------------------- |
| Service     | Yes. Only one definition can respond.                                |
| Publisher   | Yes. Only one definition can publish (to avoid inconsistent states). |
| Diagnostics | Yes. Only one definition can set the level.                          |
| Subscriber  | No. Multiple definitions can receive the same topic.                 |

To detect conflicts at startup, each definition declares the interfaces it owns in advance
(e.g. `@service` decorators, a `publishes` list in plugins).
Publishing to a topic not owned by the definition is an error.

The built-in default behavior is as follows.

| Interface   | Built-in default behavior                                     |
| ----------- | ------------------------------------------------------------- |
| Service     | Return success immediately (no transition delay, no failure). |
| Diagnostics | Publish `OK` at the rate defined in the specification.        |
| Publisher   | Do nothing.                                                   |
| Subscriber  | Receive messages only.                                        |

### Declarative behavior definition (draft)

Simple behavior can be defined declaratively without writing Python code.
Complex behavior is written as a plugin.

```yaml
services:
  /system/operation_mode/change_operation_mode:
    delay: 1.0 # [s]
    response:
      status:
        success: false
        message: injected failure
diagnostics:
  "operation_mode_transition_manager: example_status":
    level: ERROR
    message: injected error
```

### Plugin (draft)

Plugins are written in Python.
Callbacks can be normal functions or coroutines (`async def`), as supported by rclpy.

```python
from autoware_mock_common import MockPlugin, service, subscription


class FailModeChange(MockPlugin):
    publishes = ["/system/operation_mode/state"]

    def on_init(self, ctx):
        self.state = ctx.default_message("/system/operation_mode/state")
        ctx.publish("/system/operation_mode/state", self.state)

    @service("/system/operation_mode/change_operation_mode")
    def change_mode(self, ctx, req, res):
        res.status.success = False
        return res

    @subscription("/some/input")
    def on_input(self, ctx, msg):
        pass
```

The context `ctx` provides the following functions.

- Publish messages (`ctx.publish`).
- Set the level of diagnostic statuses (`ctx.set_diag(name, level, message)`).
- Create a default message of an interface (`ctx.default_message`).
- Access the external specification (`ctx.spec`) and the ROS node (`ctx.node`, e.g. for timers and clock).
- Share state between plugins (`ctx.shared`).

Plugins are specified as `module:Class` or a file path, and multiple plugins can be stacked.

```bash
ros2 run autoware_mock_common mock --ros-args \
  -p spec:=system -p plugins:="['my_test.plugins:FailModeChange', '/path/to/plugin.py']"
```

### Execution

The mock can be run in either of the following ways.

- **Separate process**: Run as a ROS node (e.g. `ros2 run`, launch files).
- **In-process**: Create and spin the mock in the same process as the test (e.g. pytest),
  so that plugins can be defined directly in the test code.

```python
from autoware_mock_common import MockNode

mock = MockNode(spec="system", plugins=[FailModeChange()])
```

The mock engine keeps the concurrency mechanism simple, and concurrency is the responsibility of plugin authors.

- The mock uses a single-threaded executor by default.
- All interfaces created by the engine belong to a reentrant callback group,
  so a coroutine callback waiting with `await` does not block other callbacks.
  (With the default mutually exclusive callback group, awaiting a timer inside a callback deadlocks.)
- The engine does not provide locks or helpers for sleeping.
  If a plugin needs delays or thread safety (e.g. when accessed from the test thread in-process),
  the plugin author implements them.

### Recording

The mock records the interactions with other components so that tests can verify them.

- Received service requests and the returned responses.
- Received messages of subscribed topics.
- Published messages (including diagnostics).

Each record has a timestamp. Tests can get and clear the records (e.g. `mock.records(name)`, `mock.clear_records()`).

### Behavior change during a test

Changing the behavior during a test (e.g. "fail from now on") may be needed in the future.
When running in-process, the test can modify plugins directly (thread safety is the responsibility of the plugin author).
When running as a separate process, a common control service that plugins can receive is an option.

## Open questions

- Details of the plugin API.
- How to change the behavior during a test when running as a separate process (control interface).
- How to get the records when running as a separate process.
- Schema of the declarative behavior definition and its relationship with the formal behavior description in the external specification.
- Formal description of behavior in the external specification files.
- Publish behavior of `/system/operation_mode/state` in the actual component (to be checked).
