# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(${CMAKE_SOURCE_DIR}/third_party/cmake/Modules/GoogleTest.cmake)

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
endfunction()

if(NOT TARGET GTest::GTest)
        add_library(GTest::GTest INTERFACE IMPORTED)
  target_link_libraries(GTest::GTest INTERFACE CONAN_PKG::gtest)
endif()