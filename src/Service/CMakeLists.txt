# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To make OrbitAsioServer and OrbitGrpcServer available for fuzz-testing
# the target OrbitServiceLib was introduced. OrbitService still exists,
# but only builds main.cpp and links to OrbitServiceLib.
project(OrbitServiceLib)

add_library(OrbitServiceLib STATIC)

target_sources(OrbitServiceLib PRIVATE
        OrbitGrpcServer.cpp
        OrbitGrpcServer.h
        OrbitService.cpp
        OrbitService.h)

target_include_directories(OrbitServiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OrbitServiceLib PUBLIC
        GrpcProtos
        OrbitVersion
        ProducerSideService
)

if(WIN32)
target_link_libraries(OrbitServiceLib PUBLIC
        WindowsCaptureService
        WindowsProcessService)
else()
target_link_libraries(OrbitServiceLib PUBLIC
        CrashService
        LinuxCaptureService
        ProcessService
        TracepointService)
endif()

project(OrbitService)
add_executable(OrbitService main.cpp)
target_link_libraries(OrbitService PRIVATE
  OrbitServiceLib
  absl::flags
  absl::flags_usage
  absl::flags_parse)

if (NOT WIN32)
install(TARGETS OrbitService
        CONFIGURATIONS Release
        RUNTIME DESTINATION /opt/orbitprofiler)
endif()
