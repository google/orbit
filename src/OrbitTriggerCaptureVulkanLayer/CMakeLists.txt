# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitTriggerCaptureVulkanLayer)
add_library(OrbitTriggerCaptureVulkanLayer SHARED)

target_sources(OrbitTriggerCaptureVulkanLayer PRIVATE
        DispatchTable.cpp
        DispatchTable.h
        LayerOptions.cpp
        LayerOptions.h
        LayerLogic.cpp
        LayerLogic.h
        OrbitCaptureClientLayer.cpp)

target_link_libraries(OrbitTriggerCaptureVulkanLayer PUBLIC
        OrbitBase
        OrbitCaptureGgpClientLib
        protobuf::protobuf
        vulkan-headers::vulkan-headers
        vulkan-validationlayers::vulkan-validationlayers)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protos/OrbitTriggerCaptureVulkanLayer)
protobuf_generate(TARGET OrbitTriggerCaptureVulkanLayer PROTOS
        layer_config.proto
        PROTOC_OUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/protos/OrbitTriggerCaptureVulkanLayer)

target_include_directories(OrbitTriggerCaptureVulkanLayer PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/protos)

iwyu_add_dependency(OrbitTriggerCaptureVulkanLayer)