# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitCaptureVulkanLayer)
add_library(OrbitCaptureVulkanLayer SHARED)

target_compile_options(OrbitCaptureVulkanLayer PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitCaptureVulkanLayer PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitCaptureVulkanLayer PRIVATE
        DispatchTable.cpp
        DispatchTable.h
        LayerData.cpp
        LayerData.h
        LayerLogic.cpp
        LayerLogic.h
        OrbitCaptureClientLayer.cpp)

target_link_libraries(OrbitCaptureVulkanLayer PUBLIC
        OrbitBase
        OrbitCaptureGgpClientLib
        OrbitCaptureVulkanLayerProtos)

project(OrbitCaptureVulkanLayerProtos)
add_library(OrbitCaptureVulkanLayerProtos STATIC)

protobuf_generate(TARGET OrbitCaptureVulkanLayerProtos PROTOS 
        layer_config.proto)

target_include_directories(OrbitCaptureVulkanLayerProtos PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR})
