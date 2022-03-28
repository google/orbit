# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ClientData)
add_library(ClientData STATIC)

target_include_directories(ClientData PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ClientData PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(ClientData PUBLIC
        include/ClientData/ApiStringEvent.h
        include/ClientData/CallstackData.h
        include/ClientData/CallstackEvent.h
        include/ClientData/CallstackInfo.h
        include/ClientData/CallstackType.h
        include/ClientData/CaptureData.h
        include/ClientData/ScopeIdConstants.h
        include/ClientData/DataManager.h
        include/ClientData/ScopeIdProvider.h
        include/ClientData/FunctionInfoSet.h
        include/ClientData/FunctionUtils.h
        include/ClientData/LinuxAddressInfo.h
        include/ClientData/ModuleAndFunctionLookup.h
        include/ClientData/ModuleData.h
        include/ClientData/ModuleManager.h
        include/ClientData/PostProcessedSamplingData.h
        include/ClientData/ProcessData.h
        include/ClientData/ScopeStats.h
        include/ClientData/ScopeTreeTimerData.h
        include/ClientData/ThreadStateSliceInfo.h
        include/ClientData/ThreadTrackDataManager.h
        include/ClientData/ThreadTrackDataProvider.h
        include/ClientData/TimerChain.h
        include/ClientData/TimerTrackDataIdManager.h
        include/ClientData/TimerData.h
        include/ClientData/TimerDataInterface.h
        include/ClientData/TimerDataManager.h
        include/ClientData/TimestampIntervalSet.h
        include/ClientData/TracepointCustom.h
        include/ClientData/TracepointData.h
        include/ClientData/UserDefinedCaptureData.h)

target_sources(ClientData PRIVATE
        CallstackData.cpp
        CallstackType.cpp
        CaptureData.cpp
        DataManager.cpp
        ScopeIdProvider.cpp
        FunctionUtils.cpp
        ModuleAndFunctionLookup.cpp
        ModuleData.cpp
        ModuleManager.cpp
        PostProcessedSamplingData.cpp
        ProcessData.cpp
        ScopeStats.cpp
        ScopeTreeTimerData.cpp
        ThreadTrackDataProvider.cpp
        TimerChain.cpp
        TimerData.cpp
        TimerTrackDataIdManager.cpp
        TimestampIntervalSet.cpp
        TracepointData.cpp
        UserDefinedCaptureData.cpp)

target_link_libraries(ClientData PUBLIC
        ApiInterface
        Containers
        ClientFlags  # TODO (b/226565085) remove when manual instrumentation grouping feature is released.
        ClientProtos
        GrpcProtos
        # TODO(b/191248550): Remove ObjectUtils once GetAbsoluteAddress is removed
        ObjectUtils
        OrbitBase
        xxHash::xxHash)


add_executable(ClientDataTests)
target_sources(ClientDataTests PRIVATE
        CallstackDataTest.cpp
        DataManagerTest.cpp
        ScopeIdProviderTest.cpp
        FunctionInfoSetTest.cpp
        ModuleDataTest.cpp
        ModuleManagerTest.cpp
        ProcessDataTest.cpp
        ScopeTreeTimerDataTest.cpp
        ThreadTrackDataManagerTest.cpp
        ThreadTrackDataProviderTest.cpp
        TimerTrackDataIdManagerTest.cpp
        TimestampIntervalSetTest.cpp
        TracepointDataTest.cpp
        TimerDataTest.cpp
        UserDefinedCaptureDataTest.cpp)

target_link_libraries(ClientDataTests PRIVATE
        ClientData
        ClientFlags # TODO (b/226565085) remove when manual instrumentation grouping feature is released.
        TestUtils
        GTest::Main)

register_test(ClientDataTests)

add_fuzzer(ModuleLoadSymbolsFuzzer ModuleLoadSymbolsFuzzer.cpp)
target_link_libraries(
        ModuleLoadSymbolsFuzzer PRIVATE ClientData
        FuzzingUtils)