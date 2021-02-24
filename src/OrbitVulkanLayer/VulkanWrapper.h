// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef ORBIT_VULKAN_LAYER_VULKAN_WRAPPER_H_
#define ORBIT_VULKAN_LAYER_VULKAN_WRAPPER_H_

#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {
class VulkanWrapper {
 public:
  VkResult CallVkEnumerateInstanceExtensionProperties(const char* layer_name,
                                                      uint32_t* property_count,
                                                      VkExtensionProperties* properties) {
    return vkEnumerateInstanceExtensionProperties(layer_name, property_count, properties);
  }
};
}  //  namespace orbit_vulkan_layer
#endif  // ORBIT_VULKAN_LAYER_VULKAN_WRAPPER_H_
