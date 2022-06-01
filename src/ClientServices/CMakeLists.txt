# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(ClientServices)
add_library(ClientServices STATIC)

target_include_directories(ClientServices PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(ClientServices PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(ClientServices PUBLIC
        include/ClientServices/CrashManager.h
        include/ClientServices/ProcessManager.h
        include/ClientServices/ProcessClient.h
        include/ClientServices/TracepointServiceClient.h
        include/ClientServices/WindowsProcessLauncherClient.h)

target_sources(ClientServices PRIVATE
        CrashManager.cpp
        ProcessManager.cpp
        ProcessClient.cpp
        TracepointServiceClient.cpp
        WindowsProcessLauncherClient.cpp)

target_link_libraries(ClientServices PUBLIC
        GrpcProtos
        Introspection
        OrbitBase)
