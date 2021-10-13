# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(WindowsTracing)

add_library(WindowsTracing STATIC)

target_compile_definitions(WindowsTracing PUBLIC WIN32_LEAN_AND_MEAN)
target_compile_definitions(WindowsTracing PUBLIC INITGUID)

target_compile_features(WindowsTracing PUBLIC cxx_std_17)

target_include_directories(WindowsTracing PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(WindowsTracing PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(WindowsTracing PUBLIC
        include/WindowsTracing/Tracer.h
        include/WindowsTracing/TracerListener.h)

target_sources(WindowsTracing PRIVATE
        ContextSwitchManager.cpp
        ContextSwitchManager.h
        EtwEventTypes.h
        KrabsTracer.cpp
        KrabsTracer.h
        Tracer.cpp
        TracerImpl.h
        TracerImpl.cpp)

target_include_directories(WindowsTracing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(WindowsTracing PUBLIC
        GrpcProtos
        ObjectUtils
        OrbitBase
        WindowsUtils
        krabsetw
        CONAN_PKG::abseil
        CONAN_PKG::grpc
        CONAN_PKG::outcome)

add_executable(WindowsTracingTests)

target_sources(WindowsTracingTests PRIVATE
        ContextSwitchManagerTest.cpp)

target_link_libraries(WindowsTracingTests PRIVATE
        WindowsTracing
        GTest::GTest
        GTest::Main)

SET_TARGET_PROPERTIES(WindowsTracingTests PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

register_test(WindowsTracingTests)


