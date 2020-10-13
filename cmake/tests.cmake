# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# The register_test function automatically registers a test with CTest.
# So it will be automatically executed in the CI and a xUnit-based XML
# report will be generated.
#
# Usage examples:
# - default:
#  register_test(OrbitQtTests)
#
# - with test isolation:
#  register_test(OrbitQtTests TEST_ISOLATION)
#
# - with custom timeout:
#  register_test(OrbitQtTests TIMEOUT 120)
#
# - combined:
#  register_test(OrbitQtTests TEST_ISOLATION TIMEOUT 120)

function(register_test TEST_TARGET)
  cmake_parse_arguments(ARGS "TEST_ISOLATION" "TIMEOUT" "" ${ARGN})
  string(REGEX REPLACE "Tests$" "" TEST_NAME "${TEST_TARGET}")

  set(TESTRESULTS_DIRECTORY "${CMAKE_BINARY_DIR}/testresults")

  if(ARGS_TEST_ISOLATION)
    set(ISOLATE "--isolate")
  else()
    set(ISOLATE "--no-isolate")
  endif()

  add_test(
    NAME "${TEST_NAME}"
    COMMAND
      "$<TARGET_FILE:${TEST_TARGET}>"
      ${ISOLATE}
      "--gtest_output=xml:${TESTRESULTS_DIRECTORY}/${TEST_NAME}_sponge_log.xml")

  if(ARGS_TIMEOUT)
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT ${ARGS_TIMEOUT})
  else()
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 30)
  endif()
endfunction()

if(NOT TARGET GTest::GTest)
  add_library(GTest::GTest INTERFACE IMPORTED)
  target_link_libraries(GTest::GTest INTERFACE CONAN_PKG::gtest)
endif()

if(NOT TARGET GTest::Main)
  set(FILE_PATH ${CMAKE_SOURCE_DIR}/third_party/aosp/gtest_main.cpp)

  add_library(GTest_Main STATIC EXCLUDE_FROM_ALL ${FILE_PATH})
  target_link_libraries(GTest_Main PUBLIC GTest::GTest)
  add_library(GTest::Main ALIAS GTest_Main)
endif()
