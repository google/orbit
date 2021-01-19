# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(QtUtils)
add_library(QtUtils STATIC)

target_compile_options(QtUtils PRIVATE ${STRICT_COMPILE_FLAGS})
target_include_directories(QtUtils PUBLIC include/)
target_link_libraries(QtUtils PUBLIC OrbitBase Qt5::Core CONAN_PKG::abseil)

target_sources(QtUtils PUBLIC include/QtUtils/FutureWatcher.h)

target_sources(QtUtils PRIVATE FutureWatcher.cpp)

set_target_properties(QtUtils PROPERTIES AUTOMOC ON)

add_executable(QtUtilsTests)
target_compile_options(QtUtilsTests PRIVATE ${STRICT_COMPILE_FLAGS})
target_sources(QtUtilsTests PRIVATE FutureWatcherTest.cpp)
target_link_libraries(QtUtilsTests PRIVATE QtUtils GTest::Main)

if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  register_test(QtUtilsTests PROPERTIES DISABLED TRUE)
endif()

if (NOT WIN32)
  register_test(QtUtilsTests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()
