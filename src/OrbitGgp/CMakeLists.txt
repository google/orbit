# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitGgp CXX)

add_library(OrbitGgp)

target_include_directories(OrbitGgp PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OrbitGgp PUBLIC
          include/OrbitGgp/Client.h
          include/OrbitGgp/Error.h
          include/OrbitGgp/Instance.h
          include/OrbitGgp/InstanceItemModel.h
          include/OrbitGgp/SshInfo.h)

target_sources(OrbitGgp PRIVATE
          Client.cpp
          Error.cpp
          Instance.cpp
          InstanceItemModel.cpp
          SshInfo.cpp)

target_link_libraries(
  OrbitGgp
  PRIVATE OrbitBase
          Qt5::Core
)

set_target_properties(OrbitGgp PROPERTIES AUTOMOC ON)
set_target_properties(OrbitGgp PROPERTIES AUTOUIC ON)

# Mock ggp clients
add_executable(OrbitMockGgpFailing)
target_sources(OrbitMockGgpFailing PRIVATE MockGgp/Failing.cpp)

add_executable(OrbitMockGgpWorking)
target_sources(OrbitMockGgpWorking PRIVATE MockGgp/Working.cpp)

# Tests
add_executable(OrbitGgpTests)

target_sources(OrbitGgpTests PRIVATE 
          ClientTest.cpp
          InstanceTests.cpp
          InstanceItemModelTests.cpp
          SshInfoTests.cpp
)

target_link_libraries(OrbitGgpTests PRIVATE 
          OrbitGgp
          OrbitBase
          GTest::QtCoreMain
          Qt5::Core
)

set_target_properties(OrbitGgpTests PROPERTIES AUTOMOC ON)
set_target_properties(OrbitGgpTests PROPERTIES AUTOUIC ON)

register_test(OrbitGgpTests)
