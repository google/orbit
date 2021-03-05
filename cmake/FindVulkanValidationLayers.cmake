# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find_path(VulkanValidationLayers_INCLUDE_DIR
  NAMES vk_layer_dispatch_table.h 
  PATH_SUFFIXES vulkan/
  HINTS "$ENV{VULKAN_SDK}/include")

set(VulkanValidationLayers_INCLUDE_DIRS ${VulkanValidationLayers_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VulkanValidationLayers DEFAULT_MSG VulkanValidationLayers_INCLUDE_DIR)

mark_as_advanced(VulkanValidationLayers_INCLUDE_DIR)

if(VulkanValidationLayers_FOUND AND NOT TARGET Vulkan::ValidationLayers)
  add_library(Vulkan::ValidationLayers INTERFACE IMPORTED)
  set_target_properties(Vulkan::ValidationLayers PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${VulkanValidationLayers_INCLUDE_DIRS}")
  target_link_libraries(Vulkan::ValidationLayers INTERFACE Vulkan::Vulkan)
endif()
