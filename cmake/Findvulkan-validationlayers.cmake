# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_package(vulkan-validationlayers CONFIG QUIET)

if(TARGET vulkan-validationlayers::vulkan-validationlayers)
  return()
endif()

message(STATUS "vulkan-validationlayers not found via CMake. Trying to find on the system...")

find_path(VULKAN_VALIDATION_INC_DIR NAMES vulkan/vk_dispatch_table_helper.h)

add_library(vulkan-validationlayers INTERFACE IMPORTED)
target_include_directories(vulkan-validationlayers INTERFACE ${VULKAN_VALIDATION_INC_DIR})
target_link_libraries(vulkan-validationlayers INTERFACE vulkan-headers::vulkan-headers)

add_library(vulkan-validationlayers::vulkan-validationlayers ALIAS vulkan-validationlayers)
