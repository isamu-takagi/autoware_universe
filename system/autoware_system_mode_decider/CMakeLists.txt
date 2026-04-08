cmake_minimum_required(VERSION 3.8)
project(autoware_system_mode_decider)

find_package(autoware_cmake REQUIRED)
autoware_package()

ament_auto_add_library(${PROJECT_NAME} SHARED
  "src/system_mode_decider.cpp"
  "src/core/decider.cpp"
  "src/core/status.cpp"
  "src/core/task.cpp"
  "src/core/mapping.cpp"
)
target_include_directories(${PROJECT_NAME} PRIVATE "src")

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "autoware::system_mode_decider::SystemModeDecider"
  EXECUTABLE "system_mode_decider_node"
)

if(BUILD_TESTING)
endif()

ament_auto_package(INSTALL_TO_SHARE config launch)
