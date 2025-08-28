# Copyright 2025 The Autoware Contributors
# Copyright 2014-2015 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

function(autoware_interface_check)

  cmake_parse_arguments(ARG "" "TESTNAME;TIMEOUT" "" ${ARGN})
  if(NOT ARG_TESTNAME)
    set(ARG_TESTNAME "autoware_interface_check")
  endif()
  if(NOT ARG_TIMEOUT)
    set(ARG_TIMEOUT 500)
  endif()

  find_program(autoware_interface_check_BIN NAMES "autoware-interface-check")
  if(NOT autoware_interface_check_BIN)
    message(FATAL_ERROR "autoware_interface_check() could not find program 'autoware-interface-check'")
  endif()

Message("================================================================================")
Message("autoware_interface_check")
Message("ARGN     : ${ARGN}")
Message("TESTNAME : ${ARG_TESTNAME}")
Message("TIMEOUT  : ${ARG_TIMEOUT}")
Message("TARGETS  : ${ARG_UNPARSED_ARGUMENTS}")
Message("================================================================================")



  set(result_file "${AMENT_TEST_RESULTS_DIR}/${PROJECT_NAME}/${ARG_TESTNAME}.xunit.xml")
  set(output_file "${CMAKE_BINARY_DIR}/autoware_interface_check/${ARG_TESTNAME}.txt")
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/autoware_interface_check")

  set(cmd "${autoware_interface_check_BIN}" "--xunit-file" "${result_file}")
  list(APPEND cmd ${ARG_UNPARSED_ARGUMENTS})


Message("result_file  : ${result_file}")
Message("output_file  : ${output_file}")
Message("cmd          : ${cmd}")
Message("================================================================================")

  ament_add_test(
    "${ARG_TESTNAME}"
    COMMAND ${cmd}
    OUTPUT_FILE "${output_file}"
    RESULT_FILE "${result_file}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    TIMEOUT "${ARG_TIMEOUT}"
  )
  set_tests_properties(
    "${ARG_TESTNAME}"
    PROPERTIES
    LABELS "autoware_interface_check"
  )

endfunction()
