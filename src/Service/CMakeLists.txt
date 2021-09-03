# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# To make OrbitAsioServer and OrbitGrpcServer available for fuzz-testing
# the target OrbitServiceLib was introduced. OrbitService still exists,
# but only builds main.cpp and links to OrbitServiceLib.
project(ServiceLib)

add_library(ServiceLib STATIC)

target_sources(ServiceLib PRIVATE
        CaptureEventBuffer.h
        CaptureEventSender.h
        CaptureServiceImpl.cpp
        CaptureServiceImpl.h
        CaptureStartStopListener.h
        MemoryInfoHandler.cpp
        MemoryInfoHandler.h
        OrbitGrpcServer.cpp
        OrbitGrpcServer.h
        OrbitService.cpp
        OrbitService.h
        Process.cpp
        Process.h
        ProcessList.cpp
        ProcessList.h
        ProcessServiceImpl.cpp
        ProcessServiceImpl.h
        ProducerEventProcessor.cpp
        ProducerEventProcessor.h
        ProducerSideServer.cpp
        ProducerSideServer.h
        ProducerSideServiceImpl.cpp
        ProducerSideServiceImpl.h
        TracingHandler.cpp
        TracingHandler.h
        ServiceUtils.cpp
        ServiceUtils.h)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set_target_properties(ServiceLib PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_include_directories(ServiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ServiceLib PUBLIC
        ApiLoader
        CrashService
        FramePointerValidatorService
        GrpcProtos
        Introspection
        LinuxTracing
        MemoryTracing
        ObjectUtils
        OrbitVersion
        ProducerSideChannel
        TracepointService
        UserSpaceInstrumentation)

project(OrbitService)
add_executable(OrbitService main.cpp)
target_link_libraries(OrbitService PRIVATE ServiceLib)

strip_symbols(OrbitService)

add_executable(ServiceTests)

target_sources(ServiceTests PRIVATE
        ProcessListTest.cpp
        ProcessTest.cpp
        ProducerEventProcessorTest.cpp
        ProducerSideServiceImplTest.cpp
        ServiceUtilsTest.cpp)

target_link_libraries(ServiceTests PRIVATE
        ServiceLib
        TestUtils
        GTest::Main)

register_test(ServiceTests PROPERTIES TIMEOUT 10)

add_fuzzer(OrbitServiceUtilsFindSymbolsFilePathFuzzer
           OrbitServiceUtilsFindSymbolsFilePathFuzzer.cpp)
target_link_libraries(
        OrbitServiceUtilsFindSymbolsFilePathFuzzer PRIVATE ServiceLib
        FuzzingUtils)
