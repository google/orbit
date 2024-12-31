# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(LinuxCaptureService)
add_library(LinuxCaptureService STATIC)

target_include_directories(LinuxCaptureService PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(LinuxCaptureService PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(LinuxCaptureService PUBLIC
        include/LinuxCaptureService/LinuxCaptureService.h
        include/LinuxCaptureService/LinuxCaptureServiceBase.h)

target_sources(LinuxCaptureService PRIVATE
        ExtractSignalFromMinidump.cpp
        ExtractSignalFromMinidump.h
        LinuxCaptureService.cpp
        LinuxCaptureServiceBase.cpp
        MemoryInfoHandler.cpp
        MemoryInfoHandler.h
        MemoryWatchdog.cpp
        MemoryWatchdog.h
        TracingHandler.cpp
        TracingHandler.h
        UserSpaceInstrumentationAddressesImpl.h)

target_link_libraries(LinuxCaptureService PUBLIC
        ApiLoader
        ApiUtils
        CaptureServiceBase
        ProducerEventProcessor
        GrpcProtos
        Introspection
        LinuxTracing
        MemoryTracing
        OrbitBase
        OrbitVersion
        UserSpaceInstrumentation)

add_executable(LinuxCaptureServiceTests)

target_sources(LinuxCaptureServiceTests PRIVATE
        ExtractSignalFromMinidumpTest.cpp
        MemoryWatchdogTest.cpp
        UserSpaceInstrumentationAddressesImplTest.cpp)

target_link_libraries(LinuxCaptureServiceTests PRIVATE
        LinuxCaptureService
        GTest_Main
        TestUtils)

register_test(LinuxCaptureServiceTests)
