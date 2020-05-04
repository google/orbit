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
        include/OrbitLinuxTracing/Events.h
        include/OrbitLinuxTracing/Function.h
        include/OrbitLinuxTracing/OrbitTracing.h
        include/OrbitLinuxTracing/Tracer.h
        include/OrbitLinuxTracing/TracerListener.h
        include/OrbitLinuxTracing/TracingOptions.h)

target_sources(OrbitLinuxTracing PRIVATE
        GpuTracepointEventProcessor.h
        GpuTracepointEventProcessor.cpp
        LibunwindstackUnwinder.cpp
        LibunwindstackUnwinder.h
        MakeUniqueForOverwrite.h
        OrbitTracing.cpp
        PerfEvent.cpp
        PerfEvent.h
        PerfEventOpen.cpp
        PerfEventOpen.h
        PerfEventProcessor.cpp
        PerfEventProcessor.h
        PerfEventProcessor2.cpp
        PerfEventProcessor2.h
        PerfEventReaders.h
        PerfEventReaders.cpp
        PerfEventRecords.h
        PerfEventRingBuffer.cpp
        PerfEventRingBuffer.h
        PerfEventVisitor.h
        Tracer.cpp
        TracerThread.cpp
        TracerThread.h
        UprobesFunctionCallManager.h
        UprobesReturnAddressManager.h
        UprobesUnwindingVisitor.cpp
        UprobesUnwindingVisitor.h
        Utils.h
        Utils.cpp)

target_link_libraries(OrbitLinuxTracing PUBLIC
        OrbitBase
        abseil::abseil
        libunwindstack::libunwindstack)

add_executable(OrbitLinuxTracingTests)

if (NOT WIN32)
    target_sources(OrbitLinuxTracingTests PRIVATE
            PerfEventProcessor2Test.cpp
            UprobesFunctionCallManagerTest.cpp
            UprobesReturnAddressManagerTest.cpp
            UtilsTest.cpp)
endif ()

target_link_libraries(OrbitLinuxTracingTests PRIVATE
        OrbitLinuxTracing
        GTest::GTest
        GTest::Main)

register_test(OrbitLinuxTracingTests)
