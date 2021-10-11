# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(QtUtils)
add_library(QtUtils STATIC)

target_include_directories(QtUtils PUBLIC include/)
target_link_libraries(QtUtils PUBLIC OrbitBase OrbitGl Qt5::Core CONAN_PKG::abseil GTest::GTest)

target_sources(QtUtils PUBLIC include/QtUtils/AssertNoQtLogWarnings.h
                              include/QtUtils/EventLoop.h
                              include/QtUtils/ExecuteProcess.h
                              include/QtUtils/FutureWatcher.h
                              include/QtUtils/MainThreadExecutorImpl.h)

target_sources(QtUtils PRIVATE ExecuteProcess.cpp
                               FutureWatcher.cpp
                               MainThreadExecutorImpl.cpp)

set_target_properties(QtUtils PROPERTIES AUTOMOC ON)

add_executable(FakeCliProgram)
target_sources(FakeCliProgram PRIVATE FakeCliProgram/main.cpp)
target_link_libraries(FakeCliProgram CONAN_PKG::abseil)

add_executable(QtUtilsTests)
target_sources(QtUtilsTests PRIVATE EventLoopTest.cpp
                                    ExecuteProcessTest.cpp
                                    FutureWatcherTest.cpp
                                    MainThreadExecutorImplTest.cpp)
target_link_libraries(QtUtilsTests PRIVATE ClientFlags TestUtils QtUtils GTest::QtCoreMain)
register_test(QtUtilsTests)
