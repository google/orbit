# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(UserSpaceInstrumentation)

add_library(UserSpaceInstrumentation STATIC)

target_include_directories(UserSpaceInstrumentation PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(UserSpaceInstrumentation PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(UserSpaceInstrumentation PUBLIC
        include/UserSpaceInstrumentation/AddressRange.h
        include/UserSpaceInstrumentation/AnyThreadIsInStrictSeccompMode.h
        include/UserSpaceInstrumentation/Attach.h
        include/UserSpaceInstrumentation/ExecuteInProcess.h
        include/UserSpaceInstrumentation/InjectLibraryInTracee.h
        include/UserSpaceInstrumentation/InstrumentProcess.h)

target_sources(UserSpaceInstrumentation PRIVATE
        AccessTraceesMemory.cpp
        AccessTraceesMemory.h
        AllocateInTracee.cpp
        AllocateInTracee.h
        AnyThreadIsInStrictSeccompMode.cpp
        Attach.cpp
        ExecuteInProcess.cpp
        ExecuteMachineCode.h
        ExecuteMachineCode.cpp
        FindFunctionAddress.h
        FindFunctionAddress.cpp
        InjectLibraryInTracee.cpp
        InstrumentProcess.cpp
        MachineCode.cpp
        MachineCode.h
        ReadSeccompModeOfThread.cpp
        ReadSeccompModeOfThread.h
        RegisterState.cpp
        RegisterState.h
        Trampoline.cpp
        Trampoline.h)

target_link_libraries(UserSpaceInstrumentation PUBLIC
        GrpcProtos
        ModuleUtils
        ObjectUtils
        OrbitBase
        absl::base
        absl::flat_hash_set
        absl::span
        absl::str_format
        absl::strings
        absl::synchronization
        absl::time
        capstone::capstone)

# This lib is injected into the target process Orbit is profiling. It is providing the functions
# executed on function entry/exit of an instrumented function.
add_library(OrbitUserSpaceInstrumentation SHARED)

# The linker flags mark all symbols coming from libraries as private and ensure that they will be hidden in the shared library.
# The visibility properties ensure that symbols coming from this module's source files are considered private unless
# explicitly marked visible in the source code file.
# This prevents functions and types that we don't want to explicitly expose from leaking into the target process and from
# potentially being used outside of this library.
set_target_properties(OrbitUserSpaceInstrumentation PROPERTIES OUTPUT_NAME "orbituserspaceinstrumentation"
                                                               LINK_FLAGS "-Wl,--exclude-libs,ALL"
                                                               C_VISIBILITY_PRESET hidden
                                                               CXX_VISIBILITY_PRESET hidden
                                                               VISIBILITY_INLINES_HIDDEN 1)

target_include_directories(OrbitUserSpaceInstrumentation PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitUserSpaceInstrumentation PRIVATE
        OrbitUserSpaceInstrumentation.cpp
        OrbitUserSpaceInstrumentation.h)

target_link_libraries(OrbitUserSpaceInstrumentation PUBLIC
        CaptureEventProducer
        OrbitBase
        ProducerSideChannel)

if (NOT WIN32)
install(TARGETS OrbitUserSpaceInstrumentation
        CONFIGURATIONS Release
        LIBRARY DESTINATION /opt/orbitprofiler)
endif()

# This test lib is merely used in UserSpaceInstrumentationTests below. The
# binary libUserSpaceInstrumentationTestLib.so created from this target is used
# to test the injection mechanism.
add_library(UserSpaceInstrumentationTestLib SHARED)

target_include_directories(UserSpaceInstrumentationTestLib PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(UserSpaceInstrumentationTestLib PRIVATE
        UserSpaceInstrumentationTestLib.cpp
        UserSpaceInstrumentationTestLib.h)

# The tests for UserSpaceInstrumentation        
add_executable(UserSpaceInstrumentationTests)

target_include_directories(UserSpaceInstrumentationTests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(UserSpaceInstrumentationTests PRIVATE
        AccessTraceesMemoryTest.cpp
        AllocateInTraceeTest.cpp
        AttachTest.cpp
        ExecuteInProcessTest.cpp
        ExecuteMachineCodeTest.cpp
        FindFunctionAddressTest.cpp
        GetTestLibLibraryPath.cpp
        GetTestLibLibraryPath.h
        InjectLibraryInTraceeTest.cpp
        InstrumentProcessTest.cpp
        MachineCodeTest.cpp
        ReadSeccompModeOfThreadTest.cpp
        RegisterStateTest.cpp
        TestProcess.cpp
        TestProcess.h
        TestUtils.cpp
        TestUtils.h
        TestUtilsTest.cpp
        TrampolineTest.cpp)

target_link_libraries(UserSpaceInstrumentationTests PRIVATE
        TestUtils
        UserSpaceInstrumentation
        capstone::capstone
        GTest::gtest
        GTest_Main)

register_test(UserSpaceInstrumentationTests)
