# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(LinuxTracing)

add_library(LinuxTracing STATIC)

target_include_directories(LinuxTracing PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(LinuxTracing PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(LinuxTracing PUBLIC
        include/LinuxTracing/Tracer.h
        include/LinuxTracing/TracerListener.h
        include/LinuxTracing/UserSpaceInstrumentationAddresses.h)

target_sources(LinuxTracing PRIVATE
        ContextSwitchManager.cpp
        ContextSwitchManager.h
        GpuTracepointVisitor.h
        GpuTracepointVisitor.cpp
        KernelTracepoints.h
        LeafFunctionCallManager.h
        LeafFunctionCallManager.cpp
        LibunwindstackMaps.cpp
        LibunwindstackMaps.h
        LibunwindstackMultipleOfflineAndProcessMemory.cpp
        LibunwindstackMultipleOfflineAndProcessMemory.h
        LibunwindstackUnwinder.cpp
        LibunwindstackUnwinder.h
        LinuxTracingUtils.h
        LinuxTracingUtils.cpp
        LostAndDiscardedEventVisitor.h
        PerfEvent.cpp
        PerfEvent.h
        PerfEventOpen.cpp
        PerfEventOpen.h
        PerfEventOrderedStream.cpp
        PerfEventOrderedStream.h
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
        TracerImpl.cpp
        TracerImpl.h
        UprobesFunctionCallManager.h
        UprobesReturnAddressManager.h
        UprobesUnwindingVisitor.cpp
        UprobesUnwindingVisitor.h)

target_link_libraries(LinuxTracing PUBLIC
        GrpcProtos
        Introspection
        libunwindstack
        ModuleUtils
        ObjectUtils
        OrbitBase
        TestUtils
        absl::flat_hash_map
        absl::flat_hash_set
        absl::meta
        absl::str_format
        absl::strings
        absl::synchronization)

add_executable(LinuxTracingTests)

target_sources(LinuxTracingTests PRIVATE
        ContextSwitchManagerTest.cpp
        GpuTracepointVisitorTest.cpp
        LeafFunctionCallManagerTest.cpp
        LibunwindstackMapsTest.cpp
        LibunwindstackMultipleOfflineAndProcessMemoryTest.cpp
        LibunwindstackUnwinderTest.cpp
        LinuxTracingUtilsTest.cpp
        LostAndDiscardedEventVisitorTest.cpp
        MockTracerListener.h
        PerfEventProcessorTest.cpp
        PerfEventQueueTest.cpp
        SwitchesStatesNamesVisitorTest.cpp
        ThreadStateManagerTest.cpp
        UprobesFunctionCallManagerTest.cpp
        UprobesReturnAddressManagerTest.cpp
        UprobesUnwindingVisitorCallchainTest.cpp
        UprobesUnwindingVisitorDwarfUnwindingTest.cpp
        UprobesUnwindingVisitorDynamicInstrumentationTest.cpp
        UprobesUnwindingVisitorMmapTest.cpp
        UprobesUnwindingVisitorTestCommon.h)

target_link_libraries(LinuxTracingTests PRIVATE
        LinuxTracing
        GTest::gtest
        GTest_Main)

register_test(LinuxTracingTests)
