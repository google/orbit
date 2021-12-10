# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To make OrbitAsioServer and OrbitGrpcServer available for fuzz-testing
# the target OrbitServiceLib was introduced. OrbitService still exists,
# but only builds main.cpp and links to OrbitServiceLib.
project(OrbitServiceLib)

add_library(OrbitServiceLib STATIC)

if(WIN32)
set(PLATFORM_DIR "Windows")
else()
set(PLATFORM_DIR "Linux")
endif()

target_sources(OrbitServiceLib PRIVATE
        OrbitGrpcServer.h
        OrbitService.h
        ProducerSideServer.cpp
        ProducerSideServer.h
        ProducerSideServiceImpl.cpp
        ProducerSideServiceImpl.h
        ${PLATFORM_DIR}/OrbitGrpcServer.cpp
        ${PLATFORM_DIR}/OrbitService.cpp)

target_include_directories(OrbitServiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OrbitServiceLib PUBLIC
        GrpcProtos
        OrbitVersion
        ProducerSideChannel
)

if(WIN32)
target_link_libraries(OrbitServiceLib PUBLIC
        WindowsCaptureService
        WindowsProcessService)
else()
target_link_libraries(OrbitServiceLib PUBLIC
        CrashService
        FramePointerValidatorService
        LinuxCaptureService
        ProcessService
        TracepointService)
endif()

project(OrbitService)
add_executable(OrbitService main.cpp)
target_link_libraries(OrbitService PRIVATE OrbitServiceLib)

strip_symbols(OrbitService)

add_executable(ServiceTests)

target_sources(ServiceTests PRIVATE
        ProducerSideServiceImplTest.cpp)

target_link_libraries(ServiceTests PRIVATE
        OrbitServiceLib
        TestUtils
        GTest::Main)

register_test(ServiceTests PROPERTIES TIMEOUT 10)
