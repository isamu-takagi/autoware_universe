cmake_minimum_required(VERSION 3.8)
project(autoware_system_mode_decider)

find_package(autoware_cmake REQUIRED)
autoware_package()

ament_auto_add_library(${PROJECT_NAME}_common SHARED
  "src/system_mode_decider.cpp"
  "src/core/config.cpp"
  "src/core/status.cpp"
  "src/core/task.cpp"
  "src/core/values.cpp"
  "src/core/decider.cpp"
)
target_include_directories(${PROJECT_NAME}_common PRIVATE
  "src"
)

ament_auto_add_library(${PROJECT_NAME}_plugin SHARED
  "src/plugin/default.cpp"
)
target_include_directories(${PROJECT_NAME}_plugin PRIVATE
  "src"
)

rclcpp_components_register_node(${PROJECT_NAME}_common
  PLUGIN "autoware::system_mode_decider::SystemModeDecider"
  EXECUTABLE "system_mode_decider_node"
)

if(BUILD_TESTING)
endif()

pluginlib_export_plugin_description_file(autoware_system_mode_decider plugins.xml)
ament_auto_package(INSTALL_TO_SHARE config launch)
