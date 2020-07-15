# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

function(register_test)
  set(TEST_TARGET "${ARGV0}")
  string(REGEX REPLACE "Tests$" "" TEST_NAME "${TEST_TARGET}")

  set(TESTRESULTS_DIRECTORY "${CMAKE_BINARY_DIR}/testresults")
  add_test(
    NAME "${TEST_NAME}"
    COMMAND
      "$<TARGET_FILE:${TEST_TARGET}>"
      "--gtest_output=xml:${TESTRESULTS_DIRECTORY}/${TEST_NAME}_sponge_log.xml")
endfunction()

if(NOT TARGET GTest::GTest)
        add_library(GTest::GTest INTERFACE IMPORTED)
  target_link_libraries(GTest::GTest INTERFACE CONAN_PKG::gtest)
endif()

if(NOT TARGET GTest::Main)
  set(FILE_PATH ${CMAKE_BINARY_DIR}/test_main.cpp)
  if(NOT EXISTS ${FILE_PATH})
    set(GTEST_MAIN_CPP "\
#include <cstdio>\n\
#include <gtest/gtest.h>\n\
\n\
int main(int argc, char* argv[]) {\n\
  printf(\"Running main() from %s\\n\", __FILE__)\;\n\
  ::testing::InitGoogleTest(&argc, argv)\;\n\
  return RUN_ALL_TESTS()\;\n\
}\n\
")

    file(WRITE ${FILE_PATH} ${GTEST_MAIN_CPP})
  endif()

  add_library(GTest_Main STATIC EXCLUDE_FROM_ALL ${FILE_PATH})
  target_link_libraries(GTest_Main PUBLIC GTest::GTest)
  add_library(GTest::Main ALIAS GTest_Main)
endif()
