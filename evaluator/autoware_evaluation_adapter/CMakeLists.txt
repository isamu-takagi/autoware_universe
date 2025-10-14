cmake_minimum_required(VERSION 3.14)
project(autoware_evaluation_adapter)

find_package(autoware_cmake REQUIRED)
autoware_package()

ament_auto_add_library(${PROJECT_NAME} SHARED
  src/interface.cpp
)

rclcpp_components_register_nodes(${PROJECT_NAME}
  "autoware::evaluation_adapter::MessageAdapter"
)

ament_auto_package(INSTALL_TO_SHARE config launch)
