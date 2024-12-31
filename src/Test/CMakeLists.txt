# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

add_library(GTest_Main OBJECT EXCLUDE_FROM_ALL test_main.cpp Path.cpp)
target_include_directories(GTest_Main PUBLIC include/)
target_link_libraries(GTest_Main PUBLIC OrbitBase GTest::gtest GTest::gmock)

add_library(GTest_QtCoreMain OBJECT EXCLUDE_FROM_ALL test_qtcore_main.cpp Path.cpp)
target_include_directories(GTest_QtCoreMain PUBLIC include/)
target_link_libraries(GTest_QtCoreMain PUBLIC OrbitBase GTest::gtest GTest::gmock Qt5::Core)
add_library(GTest::QtCoreMain ALIAS GTest_QtCoreMain)

add_library(GTest_QtGuiMain OBJECT EXCLUDE_FROM_ALL test_qtgui_main.cpp Path.cpp)
target_include_directories(GTest_QtGuiMain PUBLIC include/)
target_link_libraries(GTest_QtGuiMain PUBLIC OrbitBase GTest::gtest GTest::gmock Qt5::Widgets absl::strings)
add_library(GTest::QtGuiMain ALIAS GTest_QtGuiMain)

add_executable(TestTests StringViewTest.cpp)
target_link_libraries(TestTests PRIVATE GTest::Main)
target_compile_options(TestTests PRIVATE ${STRICT_COMPILE_FLAGS})
register_test(TestTests)