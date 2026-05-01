cmake_minimum_required(VERSION 3.14)
project(autoware_system_mode_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/DrivingModeRequest.msg"
  "msg/DrivingModeStatus.msg"
  "msg/DrivingModeStatusItem.msg"
  "msg/TrajectorySourceStatus.msg"
  "msg/CommandSourceStatus.msg"
  "msg/CommandFilterStatus.msg"
  "srv/ChangeTrajectorySource.srv"
  "srv/ChangeCommandSource.srv"
  "srv/ChangeCommandFilter.srv"
  ADD_LINTER_TESTS
  DEPENDENCIES builtin_interfaces
)

ament_auto_package()
