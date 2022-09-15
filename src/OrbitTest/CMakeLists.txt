# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitTest)

add_executable(OrbitTest)

target_sources(OrbitTest PRIVATE OrbitTest.cpp OrbitTestImpl.h OrbitTestImpl.cpp)

target_include_directories(OrbitTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(OrbitTest PRIVATE Threads::Threads ApiInterface PUBLIC OrbitBase)

if (NOT WIN32)
target_link_libraries(OrbitTest PRIVATE dl)
endif()

add_executable(OrbitTestShortLivedThreads)
target_sources(OrbitTestShortLivedThreads PRIVATE OrbitTestShortLivedThreads.cpp)
target_link_libraries(OrbitTestShortLivedThreads PRIVATE Threads::Threads)

add_executable(ThreadDependencyTestApp)
target_sources(ThreadDependencyTestApp PRIVATE
        OrbitTestThreadDependencies.cpp)
target_link_libraries(ThreadDependencyTestApp PRIVATE
        OrbitBase)
