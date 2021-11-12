# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To make OrbitAsioServer and OrbitGrpcServer available for fuzz-testing
# the target OrbitServiceLib was introduced. OrbitService still exists,
# but only builds main.cpp and links to OrbitServiceLib.
project(ServiceLib)

add_library(ServiceLib STATIC)

target_sources(ServiceLib PRIVATE
        OrbitGrpcServer.cpp
        OrbitGrpcServer.h
        OrbitService.cpp
        OrbitService.h
        ProducerSideServer.cpp
        ProducerSideServer.h
        ProducerSideServiceImpl.cpp
        ProducerSideServiceImpl.h)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(ServiceLib PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_include_directories(ServiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ServiceLib PUBLIC
        CrashService
        FramePointerValidatorService
        GrpcProtos
        LinuxCaptureService
        OrbitVersion
        ProcessService
        ProducerSideChannel
        TracepointService)

project(OrbitService)
add_executable(OrbitService main.cpp)
target_link_libraries(OrbitService PRIVATE ServiceLib)

strip_symbols(OrbitService)

add_executable(ServiceTests)

target_sources(ServiceTests PRIVATE
        ProducerSideServiceImplTest.cpp)

target_link_libraries(ServiceTests PRIVATE
        ServiceLib
        TestUtils
        GTest::Main)

register_test(ServiceTests PROPERTIES TIMEOUT 10)
