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
        include/ClientData/ApiTrackValue.h
        include/ClientData/CallstackData.h
        include/ClientData/CallstackEvent.h
        include/ClientData/CallstackInfo.h
        include/ClientData/CallstackType.h
        include/ClientData/CaptureData.h
        include/ClientData/CaptureDataHolder.h
        include/ClientData/CgroupAndProcessMemoryInfo.h
        include/ClientData/DataManager.h
        include/ClientData/FastRenderingUtils.h
        include/ClientData/FunctionInfo.h
        include/ClientData/LinuxAddressInfo.h
        include/ClientData/MockScopeIdProvider.h
        include/ClientData/MockScopeStatsCollection.h
        include/ClientData/ModuleAndFunctionLookup.h
        include/ClientData/ModuleData.h
        include/ClientData/ModuleIdentifier.h
        include/ClientData/ModuleIdentifierProvider.h
        include/ClientData/ModuleInMemory.h
        include/ClientData/ModuleManager.h
        include/ClientData/ModulePathAndBuildId.h
        include/ClientData/PageFaultsInfo.h
        include/ClientData/PostProcessedSamplingData.h
        include/ClientData/ProcessData.h
        include/ClientData/ScopeId.h
        include/ClientData/ScopeIdProvider.h
        include/ClientData/ScopeInfo.h
        include/ClientData/ScopeStats.h
        include/ClientData/ScopeStatsCollection.h
        include/ClientData/ScopeTreeTimerData.h
        include/ClientData/SystemMemoryInfo.h
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
        include/ClientData/TracepointEventInfo.h
        include/ClientData/TracepointInfo.h
        include/ClientData/UserDefinedCaptureData.h
        include/ClientData/WineSyscallHandlingMethod.h)

target_sources(ClientData PRIVATE
        CallstackData.cpp
        CallstackType.cpp
        CaptureData.cpp
        DataManager.cpp
        FunctionInfo.cpp
        ModuleAndFunctionLookup.cpp
        ModuleData.cpp
        ModuleIdentifierProvider.cpp
        ModuleManager.cpp
        PostProcessedSamplingData.cpp
        ProcessData.cpp
        ScopeIdProvider.cpp
        ScopeStats.cpp
        ScopeStatsCollection.cpp
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
        ClientProtos
        GrpcProtos
        ModuleUtils
        # TODO(b/191248550): Remove ObjectUtils once GetAbsoluteAddress is removed
        ObjectUtils
        OrbitBase
        xxHash::xxHash)

add_executable(ClientDataTests)
target_sources(ClientDataTests PRIVATE
        CallstackDataTest.cpp
        CaptureDataTest.cpp
        DataManagerTest.cpp
        FastRenderingUtilsTest.cpp
        FunctionInfoTest.cpp
        ModuleDataTest.cpp
        ModuleIdentifierTest.cpp
        ModuleIdentifierProviderTest.cpp
        ModuleManagerTest.cpp
        ModulePathAndBuildIdTest.cpp
        ProcessDataTest.cpp
        ScopeIdProviderTest.cpp
        ScopeInfoTest.cpp
        ScopeStatsCollectionTest.cpp
        ScopeTreeTimerDataTest.cpp
        ThreadTrackDataManagerTest.cpp
        ThreadTrackDataProviderTest.cpp
        TimerDataTest.cpp
        TimerTrackDataIdManagerTest.cpp
        TimestampIntervalSetTest.cpp
        TracepointDataTest.cpp
        UserDefinedCaptureDataTest.cpp)

target_link_libraries(ClientDataTests PRIVATE
        ClientData
        TestUtils
        GTest_Main)

register_test(ClientDataTests)
