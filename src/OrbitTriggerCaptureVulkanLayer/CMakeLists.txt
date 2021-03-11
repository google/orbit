# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitTriggerCaptureVulkanLayer)
add_library(OrbitTriggerCaptureVulkanLayer SHARED)

target_compile_options(OrbitTriggerCaptureVulkanLayer PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitTriggerCaptureVulkanLayer PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitTriggerCaptureVulkanLayer PRIVATE
        DispatchTable.cpp
        DispatchTable.h
        LayerOptions.cpp
        LayerOptions.h
        LayerLogic.cpp
        LayerLogic.h
        OrbitCaptureClientLayer.cpp)

target_link_libraries(OrbitTriggerCaptureVulkanLayer PUBLIC
        CONAN_PKG::vulkan-headers
        OrbitBase
        OrbitCaptureGgpClientLib
        OrbitTriggerCaptureVulkanLayerProtos
        Vulkan::ValidationLayers)

project(OrbitTriggerCaptureVulkanLayerProtos)
add_library(OrbitTriggerCaptureVulkanLayerProtos STATIC)

protobuf_generate(TARGET OrbitTriggerCaptureVulkanLayerProtos PROTOS 
        layer_config.proto)

target_include_directories(OrbitTriggerCaptureVulkanLayerProtos PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(OrbitTriggerCaptureVulkanLayerProtos PUBLIC
        CONAN_PKG::protobuf)
