# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(VulkanHeaders CONFIG QUIET)

if(TARGET vulkan-headers::vulkan-headers)
  return()
endif()

if(TARGET VulkanHeaders::VulkanHeaders)
  add_libraries(vulkan-headers::vulkan-headers ALIAS VulkanHeaders::VulkanHeaders)
  return()
endif()

message(STATUS "VulkanHeaders not found via find_package. Trying to find on system...")

find_path(VULKAN_INC_DIR NAMES vulkan/vk_layer.h REQUIRED)

add_library(VulkanHeaders INTERFACE IMPORTED)
target_include_directories(VulkanHeaders INTERFACE ${VULKAN_INC_DIR})

add_library(vulkan-headers::vulkan-headers ALIAS VulkanHeaders)
