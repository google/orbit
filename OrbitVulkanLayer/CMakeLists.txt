# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitVulkanLayer)
add_library(OrbitVulkanLayer SHARED)

target_compile_options(OrbitVulkanLayer PRIVATE ${STRICT_COMPILE_FLAGS})

target_include_directories(OrbitVulkanLayer PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(OrbitVulkanLayer PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(OrbitVulkanLayer PRIVATE
        DeviceManager.h
        DispatchTable.cpp
        DispatchTable.h
        EntryPoints.cpp
        QueueManager.cpp
        QueueManager.h
        SubmissionTracker.h
        TimerQueryPool.h
        VulkanLayerProducer.h
        VulkanLayerProducerImpl.cpp
        VulkanLayerProducerImpl.h)

target_link_libraries(OrbitVulkanLayer PUBLIC
        OrbitBase
        OrbitProducer
        OrbitProtos
        Vulkan::Vulkan
        Vulkan::ValidationLayers)

add_executable(OrbitVulkanLayerTests)

target_compile_options(OrbitVulkanLayerTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitVulkanLayerTests PRIVATE
        DeviceManagerTest.cpp
        DispatchTableTest.cpp
        SubmissionTrackerTest.cpp
        TimerQueryPoolTest.cpp
        QueueManagerTest.cpp
        VulkanLayerProducerImplTest.cpp)

target_link_libraries(
        OrbitVulkanLayerTests PRIVATE
        OrbitVulkanLayer
        GTest::Main)

register_test(OrbitVulkanLayerTests)
