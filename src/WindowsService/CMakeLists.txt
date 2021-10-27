# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To make OrbitAsioServer and OrbitGrpcServer available for fuzz-testing
# the target OrbitWindowsServiceLib was introduced. OrbitService still exists,
# but only builds main.cpp and links to OrbitWindowsServiceLib.
project(WindowsService)

add_library(WindowsServiceLib STATIC)
target_compile_options(WindowsServiceLib PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(WindowsServiceLib PRIVATE
        OrbitGrpcServer.cpp
        OrbitGrpcServer.h
        OrbitService.cpp
        OrbitService.h)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(WindowsServiceLib PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_include_directories(WindowsServiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(WindowsServiceLib PUBLIC
        GrpcProtos
        Introspection
        WindowsCaptureService
        WindowsProcessService
        WindowsTracing
        WindowsUtils
        ObjectUtils
        OrbitVersion)

project(OrbitService)
add_executable(OrbitService main.cpp)
target_link_libraries(OrbitService PRIVATE WindowsServiceLib)
target_compile_options(OrbitService PRIVATE ${STRICT_COMPILE_FLAGS})

SET_TARGET_PROPERTIES(OrbitService PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")