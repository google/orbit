# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(GTest CONFIG REQUIRED)

if(NOT TARGET GTest::gtest AND TARGET gtest::libgtest)
  add_library(GTest::gtest ALIAS gtest::libgtest)
endif()

if(NOT TARGET GTest::gmock AND TARGET gtest::gmock)
  add_library(GTest::gmock ALIAS gtest::gmock)
endif()

include(GoogleTest)

# `register_test` registers a test target with ctest. Individual test
# cases will be detected automatically and executed in separate processes.
# In addition this function takes care of collecting test results in xUNIT
# XML files and also handles the `testdata` subdirectories.
function(register_test TEST_TARGET)
  if(CMAKE_CROSSCOMPILING)
    return()
  endif()

  cmake_parse_arguments(ARGS "" "" "PROPERTIES" ${ARGN})
  set(TESTRESULTS_DIRECTORY "${CMAKE_BINARY_DIR}/testresults")

  if (NOT "TIMEOUT" IN_LIST ARGS_PROPERTIES)
    list(APPEND ARGS_PROPERTIES TIMEOUT 60)
  endif()

  gtest_discover_tests(${TEST_TARGET}
    XML_OUTPUT_DIR "${TESTRESULTS_DIRECTORY}"
    PROPERTIES ${ARGS_PROPERTIES}
    DISCOVERY_MODE PRE_TEST)

  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/testdata")
    add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E remove_directory
        $<TARGET_FILE_DIR:${TEST_TARGET}>/testdata/${TEST_TARGET}
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/testdata
        $<TARGET_FILE_DIR:${TEST_TARGET}>/testdata/${TEST_TARGET})
    target_link_libraries(${TEST_TARGET} PRIVATE OrbitBase)
  endif()
endfunction()
