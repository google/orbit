# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To make OrbitAsioServer and OrbitGrpcServer available for fuzz-testing
# the target OrbitServiceLib was introduced. OrbitService still exists,
# but only builds main.cpp and links to OrbitServiceLib.
project(OrbitServiceLib)

add_library(OrbitServiceLib STATIC)
target_compile_options(OrbitServiceLib PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitServiceLib PRIVATE
        CaptureServiceImpl.cpp
        CaptureServiceImpl.h
        CrashServiceImpl.cpp
        CrashServiceImpl.h
        FramePointerValidatorServiceImpl.cpp
        FramePointerValidatorServiceImpl.h
        OrbitGrpcServer.cpp
        OrbitGrpcServer.h
        OrbitService.cpp
        OrbitService.h
        ProcessList.cpp
        ProcessList.h
        ProcessServiceImpl.cpp
        ProcessServiceImpl.h
        Utils.cpp
        Utils.h)

if (NOT WIN32)
  target_sources(OrbitServiceLib PRIVATE
          LinuxTracingGrpcHandler.cpp
          LinuxTracingGrpcHandler.h)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(OrbitServiceLib PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_include_directories(OrbitServiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OrbitServiceLib PUBLIC
        OrbitCore
        OrbitFramePointerValidator
        OrbitLinuxTracing
        OrbitProtos
        OrbitVersion)

project(OrbitService)
add_executable(OrbitService main.cpp)
target_link_libraries(OrbitService PRIVATE OrbitServiceLib)
target_compile_options(OrbitService PRIVATE ${STRICT_COMPILE_FLAGS})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # Administrator privileges
  set_target_properties(
    OrbitService
    PROPERTIES LINK_FLAGS
               "/MANIFESTUAC:\"level='requireAdministrator' /SUBSYSTEM:WINDOWS")
endif()

strip_symbols(OrbitService)
