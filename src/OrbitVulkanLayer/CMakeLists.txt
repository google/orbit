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
        VulkanLayerController.h
        VulkanLayerProducer.h
        VulkanLayerProducerImpl.cpp
        VulkanLayerProducerImpl.h)

add_custom_command(TARGET OrbitVulkanLayer POST_BUILD COMMAND ${CMAKE_COMMAND}
                   -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/VkLayer_Orbit_implicit.json
                   $<TARGET_FILE_DIR:OrbitVulkanLayer>/VkLayer_Orbit_implicit.json)

target_link_libraries(OrbitVulkanLayer PUBLIC
        GrpcProtos
        OrbitBase
        OrbitProducer
        ProducerSideChannel
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
        VulkanLayerControllerTest.cpp
        VulkanLayerProducerImplTest.cpp)

target_link_libraries(
        OrbitVulkanLayerTests PRIVATE
        OrbitVulkanLayer
        GTest::Main)

register_test(OrbitVulkanLayerTests)
