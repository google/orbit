# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(OrbitBase)

add_library(OrbitBase STATIC)

target_compile_options(OrbitBase PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OrbitBase PUBLIC cxx_std_17)

target_include_directories(OrbitBase PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitBase PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitBase PRIVATE
        include/OrbitBase/Action.h
        include/OrbitBase/DebugUtils.h
        include/OrbitBase/Logging.h
        include/OrbitBase/MakeUniqueForOverwrite.h
        include/OrbitBase/Profiling.h
        include/OrbitBase/UniqueResource.h
        include/OrbitBase/ThreadPool.h
        include/OrbitBase/Tracing.h
        include/OrbitBase/SafeStrerror.h)

target_sources(OrbitBase PRIVATE
        Logging.cpp
        ThreadPool.cpp
        Tracing.cpp
        SafeStrerror.cpp)

target_link_libraries(OrbitBase PUBLIC
        abseil::abseil
        std::filesystem)

add_executable(OrbitBaseTests)

target_compile_options(OrbitBaseTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitBaseTests PRIVATE
    UniqueResourceTest.cpp
    ProfilingTest.cpp
    TracingTest.cpp
)

# Threadpool test contains some sleeps we couldn't work around - disable them on the CI
# since they are flaky there (but they work fine on local workstations).
if (NOT (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen"))
target_sources(OrbitBaseTests PRIVATE
    ThreadPoolTest.cpp
)
endif()

target_link_libraries(OrbitBaseTests PRIVATE
        OrbitBase
        GTest::GTest
        GTest::Main)

register_test(OrbitBaseTests)

