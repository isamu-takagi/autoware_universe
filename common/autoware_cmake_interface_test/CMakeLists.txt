cmake_minimum_required(VERSION 3.14)

project(autoware_cmake_interface_test NONE)

find_package(ament_cmake_core REQUIRED)
find_package(ament_cmake_test REQUIRED)

list(APPEND ${PROJECT_NAME}_CONFIG_EXTRAS
  "autoware_cmake_interface_test-extras.cmake"
)

install(
  DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_cmake_copyright REQUIRED)
  find_package(ament_cmake_lint_cmake REQUIRED)
  ament_copyright()
  ament_lint_cmake()
endif()
