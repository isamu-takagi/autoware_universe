# autoware_joy_controller

## Role

`autoware_joy_controller` is the package to convert a joy msg to autoware commands (e.g. steering wheel, shift, turn signal, engage) for a vehicle.

## Usage

### ROS 2 launch

```bash
# With default config (ds4)
ros2 launch autoware_joy_controller joy_controller.launch.xml

# Default config but select from the existing parameter files
ros2 launch autoware_joy_controller joy_controller_param_selection.launch.xml joy_type:=ds4 # or g29, p65, xbox

# Override the param file
ros2 launch autoware_joy_controller joy_controller.launch.xml config_file:=/path/to/your/param.yaml
```

## Input / Output

### Input topics

| Name                           | Type                                                 | Description                       |
| ------------------------------ | ---------------------------------------------------- | --------------------------------- |
| `~/input/joy`                  | sensor_msgs::msg::Joy                                | joy controller command            |
| `~/input/odometry`             | nav_msgs::msg::Odometry                              | ego vehicle odometry to get twist |
| `~/input/operation_mode_state` | autoware_adapi_v1_msgs::msg::OperationModeState      | current operation mode            |
| `~/input/control_mode_status`  | autoware_adapi_v1_msgs::msg::ManualControlModeStatus | current manual control mode       |

### Output topics

| Name                       | Type                                                 | Description             |
| -------------------------- | ---------------------------------------------------- | ----------------------- |
| `~/output/pedals`          | autoware_adapi_v1_msgs::msg::PedalsCommand           | pedals command          |
| `~/output/steering`        | autoware_adapi_v1_msgs::msg::SteeringCommand         | steering command        |
| `~/output/acceleration`    | autoware_adapi_v1_msgs::msg::AccelerationCommand     | acceleration command    |
| `~/output/velocity`        | autoware_adapi_v1_msgs::msg::VelocityCommand         | velocity command        |
| `~/output/gear`            | autoware_adapi_v1_msgs::msg::GearCommand             | gear command            |
| `~/output/turn_indicators` | autoware_adapi_v1_msgs::msg::TurnIndicatorsCommand   | turn indicators command |
| `~/output/hazard_lights`   | autoware_adapi_v1_msgs::msg::HazardLightsCommand     | hazard lights command   |
| `~/output/heartbeat`       | autoware_adapi_v1_msgs::msg::ManualOperatorHeartbeat | operator heartbeat      |

## Parameters

| Parameter                 | Type   | Description                                                                                                        |
| ------------------------- | ------ | ------------------------------------------------------------------------------------------------------------------ |
| `joy_type`                | string | joy controller type (default: DS4)                                                                                 |
| `update_rate`             | double | update rate to publish control commands                                                                            |
| `accel_ratio`             | double | ratio to calculate acceleration (commanded acceleration is ratio \* operation)                                     |
| `brake_ratio`             | double | ratio to calculate deceleration (commanded acceleration is -ratio \* operation)                                    |
| `steer_ratio`             | double | ratio to calculate deceleration (commanded steer is ratio \* operation)                                            |
| `steering_angle_velocity` | double | steering angle velocity for operation                                                                              |
| `accel_sensitivity`       | double | sensitivity to calculate acceleration for external API (commanded acceleration is pow(operation, 1 / sensitivity)) |
| `brake_sensitivity`       | double | sensitivity to calculate deceleration for external API (commanded acceleration is pow(operation, 1 / sensitivity)) |
| `raw_control`             | bool   | skip input odometry if true                                                                                        |
| `velocity_gain`           | double | ratio to calculate velocity by acceleration                                                                        |
| `max_forward_velocity`    | double | absolute max velocity to go forward                                                                                |
| `max_backward_velocity`   | double | absolute max velocity to go backward                                                                               |
| `backward_accel_ratio`    | double | ratio to calculate deceleration (commanded acceleration is -ratio \* operation)                                    |

## P65 Joystick Key Map

| Action               | Button                |
| -------------------- | --------------------- |
| Acceleration         | R2                    |
| Brake                | L2                    |
| Steering             | Left Stick Left Right |
| Shift up             | Cursor Up             |
| Shift down           | Cursor Down           |
| Shift Drive          | Cursor Left           |
| Shift Reverse        | Cursor Right          |
| Turn Signal Left     | L1                    |
| Turn Signal Right    | R1                    |
| Clear Turn Signal    | A                     |
| Switch Autonomous    | B                     |
| Emergency Stop       | Select                |
| Clear Emergency Stop | Start                 |
| Autoware Engage      | X                     |
| Autoware Disengage   | Y                     |
| Vehicle Engage       | PS                    |
| Vehicle Disengage    | Right Trigger         |

## DS4 Joystick Key Map

| Action               | Button                     |
| -------------------- | -------------------------- |
| Acceleration         | R2, ×, or Right Stick Up   |
| Brake                | L2, □, or Right Stick Down |
| Steering             | Left Stick Left Right      |
| Shift up             | Cursor Up                  |
| Shift down           | Cursor Down                |
| Shift Drive          | Cursor Left                |
| Shift Reverse        | Cursor Right               |
| Turn Signal Left     | L1                         |
| Turn Signal Right    | R1                         |
| Clear Turn Signal    | SHARE                      |
| Switch Autonomous    | OPTIONS                    |
| Emergency Stop       | PS                         |
| Clear Emergency Stop | PS                         |
| Autoware Engage      | ○                          |
| Autoware Disengage   | ○                          |
| Vehicle Engage       | △                          |
| Vehicle Disengage    | △                          |

## XBOX Joystick Key Map

| Action               | Button                |
| -------------------- | --------------------- |
| Acceleration         | RT                    |
| Brake                | LT                    |
| Steering             | Left Stick Left Right |
| Shift up             | Cursor Up             |
| Shift down           | Cursor Down           |
| Shift Drive          | Cursor Left           |
| Shift Reverse        | Cursor Right          |
| Turn Signal Left     | LB                    |
| Turn Signal Right    | RB                    |
| Clear Turn Signal    | A                     |
| Switch Autonomous    | B                     |
| Emergency Stop       | View                  |
| Clear Emergency Stop | Menu                  |
| Autoware Engage      | X                     |
| Autoware Disengage   | Y                     |
| Vehicle Engage       | Left Stick Button     |
| Vehicle Disengage    | Right Stick Button    |
