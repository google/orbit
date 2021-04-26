# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitVulkanLayer)

add_library(OrbitVulkanLayerInterface STATIC)

target_compile_options(OrbitVulkanLayerInterface PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitVulkanLayerInterface PRIVATE
        DeviceManager.h
        DispatchTable.cpp
        DispatchTable.h
        QueueManager.cpp
        QueueManager.h
        SubmissionTracker.h
        TimerQueryPool.h
        VulkanLayerController.h
        VulkanLayerProducer.h
        VulkanLayerProducerImpl.cpp
        VulkanLayerProducerImpl.h
        VulkanWrapper.h)

target_link_libraries(OrbitVulkanLayerInterface PUBLIC
        CaptureEventProducer
        CONAN_PKG::vulkan-headers
        GrpcProtos
        OrbitBase
        ProducerSideChannel
        Vulkan::ValidationLayers)

add_library(OrbitVulkanLayer SHARED)

target_compile_options(OrbitVulkanLayer PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitVulkanLayer PRIVATE
        EntryPoints.cpp)

add_custom_command(TARGET OrbitVulkanLayer POST_BUILD COMMAND ${CMAKE_COMMAND}
                   -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/VkLayer_Orbit_implicit.json
                   $<TARGET_FILE_DIR:OrbitVulkanLayer>/VkLayer_Orbit_implicit.json)

target_link_libraries(OrbitVulkanLayer PRIVATE
        OrbitVulkanLayerInterface)

strip_symbols(OrbitVulkanLayer)

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
        OrbitVulkanLayerInterface
        GTest::Main)

register_test(OrbitVulkanLayerTests)
