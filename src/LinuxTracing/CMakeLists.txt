# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(LinuxTracing)

add_library(LinuxTracing STATIC)

target_compile_options(LinuxTracing PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(LinuxTracing PUBLIC cxx_std_17)

target_include_directories(LinuxTracing PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(LinuxTracing PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(LinuxTracing PUBLIC
        include/LinuxTracing/Tracer.h
        include/LinuxTracing/TracerListener.h)

target_sources(LinuxTracing PRIVATE
        ContextSwitchManager.cpp
        ContextSwitchManager.h
        Function.h
        GpuTracepointVisitor.h
        GpuTracepointVisitor.cpp
        KernelTracepoints.h
        LibunwindstackUnwinder.cpp
        LibunwindstackUnwinder.h
        LinuxTracingUtils.h
        LinuxTracingUtils.cpp
        ManualInstrumentationConfig.h
        PerfEvent.cpp
        PerfEvent.h
        PerfEventOpen.cpp
        PerfEventOpen.h
        PerfEventProcessor.cpp
        PerfEventProcessor.h
        PerfEventQueue.cpp
        PerfEventQueue.h
        PerfEventReaders.h
        PerfEventReaders.cpp
        PerfEventRecords.h
        PerfEventRingBuffer.cpp
        PerfEventRingBuffer.h
        PerfEventVisitor.h
        SwitchesStatesNamesVisitor.cpp
        SwitchesStatesNamesVisitor.h
        ThreadStateManager.cpp
        ThreadStateManager.h
        Tracer.cpp
        TracerThread.cpp
        TracerThread.h
        UprobesFunctionCallManager.h
        UprobesReturnAddressManager.h
        UprobesUnwindingVisitor.cpp
        UprobesUnwindingVisitor.h)

target_link_libraries(LinuxTracing PUBLIC
        ElfUtils
        GrpcProtos
        OrbitBase
        CONAN_PKG::abseil
        CONAN_PKG::libunwindstack)

add_executable(LinuxTracingTests)

target_compile_options(LinuxTracingTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(LinuxTracingTests PRIVATE
        ContextSwitchManagerTest.cpp
        GpuTracepointVisitorTest.cpp
        LinuxTracingUtilsTest.cpp
        PerfEventProcessorTest.cpp
        PerfEventQueueTest.cpp
        ThreadStateManagerTest.cpp
        UprobesFunctionCallManagerTest.cpp
        UprobesReturnAddressManagerTest.cpp)

target_link_libraries(LinuxTracingTests PRIVATE
        LinuxTracing
        GTest::GTest
        GTest::Main)

register_test(LinuxTracingTests)

add_library(LinuxTracingIntegrationTestPuppetSharedObject SHARED)

target_compile_options(LinuxTracingIntegrationTestPuppetSharedObject PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(LinuxTracingIntegrationTestPuppetSharedObject PRIVATE
        LinuxTracingIntegrationTestPuppetSharedObject.cpp)

target_link_libraries(LinuxTracingIntegrationTestPuppetSharedObject PUBLIC
        dl
        OrbitBase)

add_executable(LinuxTracingIntegrationTests)

target_compile_options(LinuxTracingIntegrationTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(LinuxTracingIntegrationTests PRIVATE
        LinuxTracingIntegrationTest.cpp
        LinuxTracingIntegrationTestPuppet.cpp
        LinuxTracingIntegrationTestPuppet.h)

target_link_libraries(LinuxTracingIntegrationTests PRIVATE
        ElfUtils
        LinuxTracing
        OrbitBase
        GTest::GTest
        GTest::Main
        CONAN_PKG::abseil)

register_test(LinuxTracingIntegrationTests)
