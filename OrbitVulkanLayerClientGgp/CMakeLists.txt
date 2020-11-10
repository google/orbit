# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitVulkanLayerClientGgp)
add_library(OrbitVulkanLayerClientGgp SHARED)

target_compile_options(OrbitVulkanLayerClientGgp PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitVulkanLayerClientGgp PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitVulkanLayerClientGgp PRIVATE
        DispatchTable.cpp
        DispatchTable.h
        LayerLogic.cpp
        LayerLogic.h
        LayerTimes.h
        OrbitCaptureClientLayer.cpp)

target_link_libraries(OrbitVulkanLayerClientGgp PUBLIC
        OrbitBase
        OrbitCaptureGgpClientLib)