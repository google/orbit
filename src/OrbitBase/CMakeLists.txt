# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitBase)

add_library(OrbitBase STATIC)

target_include_directories(OrbitBase PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitBase PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitBase PRIVATE
        include/OrbitBase/Action.h
        include/OrbitBase/Align.h
        include/OrbitBase/AnyErrorOf.h
        include/OrbitBase/AnyInvocable.h
        include/OrbitBase/AnyMovable.h
        include/OrbitBase/Append.h
        include/OrbitBase/Attributes.h
        include/OrbitBase/CanceledOr.h
        include/OrbitBase/Chunk.h
        include/OrbitBase/ExecutablePath.h
        include/OrbitBase/ExecuteCommand.h
        include/OrbitBase/Executor.h
        include/OrbitBase/File.h
        include/OrbitBase/Future.h
        include/OrbitBase/FutureHelpers.h
        include/OrbitBase/Logging.h
        include/OrbitBase/MakeUniqueForOverwrite.h
        include/OrbitBase/NotFoundOr.h
        include/OrbitBase/GetProcessIds.h
        include/OrbitBase/Overloaded.h
        include/OrbitBase/ParameterPackTrait.h
        include/OrbitBase/Profiling.h
        include/OrbitBase/Promise.h
        include/OrbitBase/PromiseHelpers.h
        include/OrbitBase/ReadFileToString.h
        include/OrbitBase/SafeStrerror.h
        include/OrbitBase/SharedState.h
        include/OrbitBase/SimpleExecutor.h
        include/OrbitBase/Sort.h
        include/OrbitBase/StringConversion.h
        include/OrbitBase/StopSource.h
        include/OrbitBase/StopToken.h
        include/OrbitBase/TaskGroup.h
        include/OrbitBase/ThreadConstants.h
        include/OrbitBase/ThreadPool.h
        include/OrbitBase/ThreadUtils.h
        include/OrbitBase/Typedef.h
        include/OrbitBase/TypedefUtils.h
        include/OrbitBase/UniqueResource.h
        include/OrbitBase/VoidToMonostate.h
        include/OrbitBase/WhenAll.h
        include/OrbitBase/WhenAny.h
        include/OrbitBase/WriteStringToFile.h)

target_sources(OrbitBase PRIVATE
        ExecutablePath.cpp
        File.cpp
        Logging.cpp
        LoggingUtils.cpp
        Profiling.cpp
        ReadFileToString.cpp
        SafeStrerror.cpp
        SimpleExecutor.cpp
        StringConversion.cpp
        ThreadPool.cpp
        WhenAll.cpp
        WriteStringToFile.cpp)

if (WIN32)
target_sources(OrbitBase PRIVATE
        include/OrbitBase/GetLastError.h
        include/OrbitBase/GetProcAddress.h
        include/OrbitBase/OsVersionWindows.h)

target_sources(OrbitBase PRIVATE
        ExecutablePathWindows.cpp
        GetLastErrorWindows.cpp
        GetProcAddressWindows.cpp
        LoggingWindows.cpp
        OsVersionWindows.cpp
        ThreadUtilsWindows.cpp)
else()
target_sources(OrbitBase PRIVATE
        ExecutablePathLinux.cpp
        ExecuteCommandLinux.cpp
        GetProcessIdsLinux.cpp
        ThreadUtilsLinux.cpp)
endif()

target_link_libraries(OrbitBase PUBLIC
        outcome::outcome
        absl::base
        absl::flat_hash_map
        absl::strings
        absl::str_format
        absl::synchronization
        absl::time
        std::filesystem)

add_executable(OrbitBaseTests)

target_sources(OrbitBaseTests PRIVATE
        AlignTest.cpp
        AnyInvocableTest.cpp
        AnyErrorOfTest.cpp
        AnyMovableTest.cpp
        AppendTest.cpp
        CanceledOrTest.cpp
        ChunkTest.cpp
        ExecutablePathTest.cpp
        ExecutorTest.cpp
        FileTest.cpp
        FutureTest.cpp
        FutureHelpersTest.cpp
        ImmediateExecutorTest.cpp
        LoggingUtilsTest.cpp
        NotFoundOrTest.cpp
        OverloadedTest.cpp
        ParameterPackTraitTest.cpp
        ProfilingTest.cpp
        PromiseTest.cpp
        PromiseHelpersTest.cpp
        ReadFileToStringTest.cpp
        ResultTest.cpp
        SimpleExecutorTest.cpp
        SortTest.cpp
        StringConversionTest.cpp
        StopSourceTest.cpp
        StopTokenTest.cpp
        TaskGroupTest.cpp
        TypedefTest.cpp
        ThreadUtilsTest.cpp
        UniqueResourceTest.cpp
        WhenAllTest.cpp
        WhenAnyTest.cpp
        WriteStringToFileTest.cpp
)

if (WIN32)
target_sources(OrbitBaseTests PRIVATE
        GetProcAddressTest.cpp
        OsVersionWindowsTest.cpp)
else()
target_sources(OrbitBaseTests PRIVATE
        ExecutablePathLinuxTest.cpp
        ExecuteCommandLinuxTest.cpp
        GetProcessIdsLinuxTest.cpp)
endif()

# Threadpool test contains some sleeps we couldn't work around - disable them on the CI
# since they are flaky there (but they work fine on local workstations).
if (NOT (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen"))
target_sources(OrbitBaseTests PRIVATE
        ThreadPoolTest.cpp
)
endif()

target_link_libraries(OrbitBaseTests PRIVATE
        OrbitBase
        TestUtils
        GTest::gtest
        GTest::Main)

register_test(OrbitBaseTests)