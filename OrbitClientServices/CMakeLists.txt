# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitClientServices)
add_library(OrbitClientServices STATIC TracepointManager.cpp include/OrbitClientServices/TracepointManager.h)

target_compile_options(OrbitClientServices PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitClientServices PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitClientServices PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitClientServices PUBLIC 
        include/OrbitClientServices/CrashManager.h
        include/OrbitClientServices/ProcessManager.h
        include/OrbitClientServices/ProcessClient.h)

target_sources(OrbitClientServices PRIVATE
        CrashManager.cpp
        ProcessManager.cpp
        ProcessClient.cpp)

target_link_libraries(OrbitClientServices PUBLIC
        OrbitBase
        OrbitCore
        OrbitProtos)