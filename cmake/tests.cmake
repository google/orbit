# Copyright (c) 2021 The Orbit Authors. All rights reserved.
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

if(NOT TARGET GTest::QtCoreMain AND TARGET Qt5::Core)
  set(FILE_PATH ${CMAKE_BINARY_DIR}/test_qtcore_main.cpp)
  if(NOT EXISTS ${FILE_PATH})
    set(GTEST_MAIN_CPP "\
#include <cstdio>\n\
#include <gtest/gtest.h>\n\
#include <QCoreApplication>\n\
\n\
int main(int argc, char* argv[]) {\n\
  QCoreApplication app{argc, argv}\;\n\
  printf(\"Running main() from %s\\n\", __FILE__)\;\n\
  ::testing::InitGoogleTest(&argc, argv)\;\n\
  return RUN_ALL_TESTS()\;\n\
}\n\
")

    file(WRITE ${FILE_PATH} ${GTEST_MAIN_CPP})
  endif()

  add_library(GTest_QtCoreMain STATIC EXCLUDE_FROM_ALL ${FILE_PATH})
  target_link_libraries(GTest_QtCoreMain PUBLIC GTest::GTest Qt5::Core)
  add_library(GTest::QtCoreMain ALIAS GTest_QtCoreMain)
endif()

if(NOT TARGET GTest::QtGuiMain AND TARGET Qt5::Widgets)
  set(FILE_PATH ${CMAKE_BINARY_DIR}/test_qtgui_main.cpp)
  if(NOT EXISTS ${FILE_PATH})
    set(GTEST_MAIN_CPP "\
#include <cstdio>\n\
#include <gtest/gtest.h>\n\
#include <QApplication>\n\
\n\
int main(int argc, char* argv[]) {\n\
  QApplication app{argc, argv}\;\n\
  printf(\"Running main() from %s\\n\", __FILE__)\;\n\
  ::testing::InitGoogleTest(&argc, argv)\;\n\
  return RUN_ALL_TESTS()\;\n\
}\n\
")

    file(WRITE ${FILE_PATH} ${GTEST_MAIN_CPP})
  endif()

  add_library(GTest_QtGuiMain STATIC EXCLUDE_FROM_ALL ${FILE_PATH})
  target_link_libraries(GTest_QtGuiMain PUBLIC GTest::GTest Qt5::Widgets)
  add_library(GTest::QtGuiMain ALIAS GTest_QtGuiMain)
endif()