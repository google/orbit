# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.15)

project(VulkanTutorial)


add_library(OffscreenRenderingVulkanTutorialLib STATIC)

target_compile_options(OffscreenRenderingVulkanTutorialLib PRIVATE ${STRICT_COMPILE_FLAGS})

target_compile_features(OffscreenRenderingVulkanTutorialLib PUBLIC cxx_std_17)

target_include_directories(OffscreenRenderingVulkanTutorialLib PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(OffscreenRenderingVulkanTutorialLib PUBLIC
        include/VulkanTutorial/OffscreenRenderingVulkanTutorial.h)

target_sources(OffscreenRenderingVulkanTutorialLib PRIVATE
        OffscreenRenderingVulkanTutorial.cpp
        VulkanTutorialFragmentShader.h
        VulkanTutorialVertexShader.h)

target_link_libraries(OffscreenRenderingVulkanTutorialLib PUBLIC
        OrbitBase
        CONAN_PKG::volk)


add_executable(OffscreenRenderingVulkanTutorial)

target_compile_options(OffscreenRenderingVulkanTutorial PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OffscreenRenderingVulkanTutorial PRIVATE
        OffscreenRenderingVulkanTutorialMain.cpp)

target_link_libraries(OffscreenRenderingVulkanTutorial PRIVATE
        OrbitBase
        OffscreenRenderingVulkanTutorialLib)
