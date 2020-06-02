# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitSshQt)

add_library(OrbitSshQt STATIC)

target_compile_options(OrbitSshQt PRIVATE ${STRICT_COMPILE_FLAGS})
target_compile_features(OrbitSshQt PUBLIC cxx_std_17)
target_include_directories(OrbitSshQt PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(
  OrbitSshQt
  PUBLIC include/OrbitSshQt/StateMachineHelper.h
  include/OrbitSshQt/Error.h
  include/OrbitSshQt/Task.h
  include/OrbitSshQt/Tunnel.h
  include/OrbitSshQt/ScopedConnection.h
  include/OrbitSshQt/SftpChannel.h
  include/OrbitSshQt/SftpOperation.h
  include/OrbitSshQt/Session.h)

target_sources(
  OrbitSshQt
  PRIVATE Error.cpp Session.cpp Tunnel.cpp Task.cpp SftpChannel.cpp SftpOperation.cpp)

target_link_libraries(OrbitSshQt PUBLIC OrbitBase OrbitSsh Qt5::Core Qt5::Network abseil::abseil)
set_target_properties(OrbitSshQt PROPERTIES AUTOMOC ON)

add_executable(OrbitSshQtIntegrationTests)
target_compile_options(OrbitSshQtIntegrationTests PRIVATE ${STRICT_COMPILE_FLAGS})
target_sources(OrbitSshQtIntegrationTests PRIVATE IntegrationTests.cpp)
target_link_libraries(OrbitSshQtIntegrationTests PUBLIC OrbitSshQt GTest::GTest)

# tests
#add_executable(OrbitSshQtTests)
#target_sources(OrbitSshQtTests PRIVATE SocketTests.cpp ContextTests.cpp)
#target_link_libraries(OrbitSshQtTests PRIVATE OrbitSshQt GTest::Main)

#register_test(OrbitSshQtTests)
