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
