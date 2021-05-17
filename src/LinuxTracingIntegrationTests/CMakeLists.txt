# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(LinuxTracingIntegrationTests)

add_library(LinuxTracingIntegrationTestPuppetSharedObject SHARED)

target_compile_options(LinuxTracingIntegrationTestPuppetSharedObject PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(LinuxTracingIntegrationTestPuppetSharedObject PRIVATE
        LinuxTracingIntegrationTestPuppetSharedObject.cpp)

target_link_libraries(LinuxTracingIntegrationTestPuppetSharedObject PUBLIC
        dl
        OrbitBase)

strip_symbols(LinuxTracingIntegrationTestPuppetSharedObject)

add_executable(LinuxTracingIntegrationTests)

target_compile_options(LinuxTracingIntegrationTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(LinuxTracingIntegrationTests PRIVATE
        LinuxTracingIntegrationTest.cpp
        LinuxTracingIntegrationTestPuppet.cpp
        LinuxTracingIntegrationTestPuppet.h)

target_link_libraries(LinuxTracingIntegrationTests PRIVATE
        GrpcProtos
        LinuxTracing
        ObjectUtils
        OffscreenRenderingVulkanTutorialLib
        OrbitBase
        GTest::GTest
        GTest::Main
        CONAN_PKG::abseil)

strip_symbols(LinuxTracingIntegrationTests)

register_test(LinuxTracingIntegrationTests)
