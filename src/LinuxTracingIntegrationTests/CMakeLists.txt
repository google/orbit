# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)
project(LinuxTracingIntegrationTests)


add_library(IntegrationTestPuppetSharedObject SHARED)

target_sources(IntegrationTestPuppetSharedObject PRIVATE
        IntegrationTestPuppetSharedObject.cpp)

target_link_libraries(IntegrationTestPuppetSharedObject PUBLIC
        dl
        OrbitBase)


add_library(IntegrationTestCommons STATIC)

target_include_directories(IntegrationTestCommons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(IntegrationTestCommons PRIVATE
        IntegrationTestChildProcess.h
        IntegrationTestCommons.cpp
        IntegrationTestCommons.h
        IntegrationTestPuppet.cpp
        IntegrationTestPuppet.h
        IntegrationTestUtils.cpp
        IntegrationTestUtils.h)

target_link_libraries(IntegrationTestCommons PUBLIC
        ApiInterface
        GrpcProtos
        ModuleUtils
        ObjectUtils
        OrbitBase
        GTest::gtest
        absl::base
        absl::time
        absl::flat_hash_set
        absl::strings
        absl::synchronization)

if(WITH_VULKAN)
  target_link_libraries(IntegrationTestCommons PUBLIC OffscreenRenderingVulkanTutorialLib)
endif()


add_executable(LinuxTracingIntegrationTests)

target_sources(LinuxTracingIntegrationTests PRIVATE
        LinuxTracingIntegrationTest.cpp)

target_link_libraries(LinuxTracingIntegrationTests PRIVATE
        GrpcProtos
        IntegrationTestCommons
        LinuxTracing
        ObjectUtils
        OrbitBase
        GTest::gtest
        GTest_Main
        absl::base
        absl::time
        absl::flat_hash_set
        absl::strings
        absl::synchronization)

register_test(LinuxTracingIntegrationTests)


add_executable(OrbitServiceIntegrationTests)

target_sources(OrbitServiceIntegrationTests PRIVATE
        OrbitServiceIntegrationTest.cpp
        IntegrationTestPuppet.cpp
        IntegrationTestPuppet.h)

target_link_libraries(OrbitServiceIntegrationTests PRIVATE
        ApiInterface
        ApiUtils
        GrpcProtos
        IntegrationTestCommons
        ObjectUtils
        OrbitBase
        OrbitServiceLib
        OrbitVersion
        GTest::gtest
        GTest_Main
        absl::base
        absl::time
        absl::flat_hash_set
        absl::strings
        absl::synchronization)

register_test(OrbitServiceIntegrationTests)
