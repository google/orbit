# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitVulkanLayer)

add_library(OrbitVulkanLayerInterface STATIC)

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
        GrpcProtos
        OrbitBase
        ProducerSideChannel
        vulkan-headers::vulkan-headers
        vulkan-validationlayers::vulkan-validationlayers)

add_library(OrbitVulkanLayer SHARED)

target_sources(OrbitVulkanLayer PRIVATE
        EntryPoints.cpp)

# The linker flags mark all symbols coming from libraries as private and ensure that they will be hidden in the shared library.
# The visibility properties ensure that symbols coming from this module's source files are considered private unless
# explicitly marked visible in the source code file.
# This prevents functions and types that we don't want to explicitly expose from leaking into the target process and from
# potentially being used outside of this library.
# Functions that may not be hidden, have to be marked with the `VK_LAYER_EXPORT` macro.
set_target_properties(OrbitVulkanLayer PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,ALL"
                                                  C_VISIBILITY_PRESET hidden
                                                  CXX_VISIBILITY_PRESET hidden
                                                  VISIBILITY_INLINES_HIDDEN 1)

add_custom_command(TARGET OrbitVulkanLayer POST_BUILD COMMAND ${CMAKE_COMMAND}
                   -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/VkLayer_Orbit_implicit.json
                   $<TARGET_FILE_DIR:OrbitVulkanLayer>/VkLayer_Orbit_implicit.json)

target_link_libraries(OrbitVulkanLayer PRIVATE
        OrbitVulkanLayerInterface)

if (NOT WIN32)
install(TARGETS OrbitVulkanLayer
        CONFIGURATIONS Release
        LIBRARY DESTINATION /opt/orbitprofiler)
endif()

add_executable(OrbitVulkanLayerTests)

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
        FakeProducerSideService
        OrbitVulkanLayerInterface
        GTest_Main)

register_test(OrbitVulkanLayerTests)
