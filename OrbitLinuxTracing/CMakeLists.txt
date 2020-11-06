# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitLinuxTracing)

add_library(OrbitLinuxTracing STATIC)

target_compile_options(OrbitLinuxTracing PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitLinuxTracing PUBLIC cxx_std_17)

target_include_directories(OrbitLinuxTracing PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitLinuxTracing PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitLinuxTracing PUBLIC
        include/OrbitLinuxTracing/OrbitTracing.h
        include/OrbitLinuxTracing/Tracer.h
        include/OrbitLinuxTracing/TracerListener.h)

target_sources(OrbitLinuxTracing PRIVATE
        ContextSwitchManager.cpp
        ContextSwitchManager.h
        Function.h
        GpuTracepointEventProcessor.h
        GpuTracepointEventProcessor.cpp
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
        ThreadStateManager.cpp
        ThreadStateManager.h
        ThreadStateVisitor.cpp
        ThreadStateVisitor.h
        Tracer.cpp
        TracerThread.cpp
        TracerThread.h
        UprobesFunctionCallManager.h
        UprobesReturnAddressManager.h
        UprobesUnwindingVisitor.cpp
        UprobesUnwindingVisitor.h)

target_link_libraries(OrbitLinuxTracing PUBLIC
        ElfUtils
        OrbitBase
        OrbitProtos
        CONAN_PKG::abseil
        CONAN_PKG::libunwindstack)

add_executable(OrbitLinuxTracingTests)

target_compile_options(OrbitLinuxTracingTests PRIVATE ${STRICT_COMPILE_FLAGS})

if (NOT WIN32)
    target_sources(OrbitLinuxTracingTests PRIVATE
            ContextSwitchManagerTest.cpp
            GpuTracepointEventProcessorTest.cpp
            LinuxTracingUtilsTest.cpp
            PerfEventProcessorTest.cpp
            PerfEventQueueTest.cpp
            ThreadStateManagerTest.cpp
            UprobesFunctionCallManagerTest.cpp
            UprobesReturnAddressManagerTest.cpp)
endif()

target_link_libraries(OrbitLinuxTracingTests PRIVATE
        OrbitLinuxTracing
        GTest::GTest
        GTest::Main)

register_test(OrbitLinuxTracingTests)
