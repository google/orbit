# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(VulkanTutorial)


add_library(OffscreenRenderingVulkanTutorialLib STATIC)

target_include_directories(OffscreenRenderingVulkanTutorialLib PUBLIC
        ${CMAKE_SOURCE_DIR}/third_party/VulkanTutorial/include)

target_sources(OffscreenRenderingVulkanTutorialLib PUBLIC
        ${CMAKE_SOURCE_DIR}/third_party/VulkanTutorial/include/VulkanTutorial/OffscreenRenderingVulkanTutorial.h)

target_sources(OffscreenRenderingVulkanTutorialLib PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/VulkanTutorial/OffscreenRenderingVulkanTutorial.cpp
        ${CMAKE_SOURCE_DIR}/third_party/VulkanTutorial/VulkanTutorialFragmentShader.h
        ${CMAKE_SOURCE_DIR}/third_party/VulkanTutorial/VulkanTutorialVertexShader.h)

target_link_libraries(OffscreenRenderingVulkanTutorialLib PUBLIC
        OrbitBase
        volk::volk)


add_executable(OffscreenRenderingVulkanTutorial)

target_sources(OffscreenRenderingVulkanTutorial PRIVATE
        OffscreenRenderingVulkanTutorialMain.cpp)

target_link_libraries(OffscreenRenderingVulkanTutorial PRIVATE
        OrbitBase
        OffscreenRenderingVulkanTutorialLib)
