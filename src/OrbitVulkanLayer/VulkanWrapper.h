// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_VULKAN_WRAPPER_H_
#define ORBIT_VULKAN_LAYER_VULKAN_WRAPPER_H_

#include <vulkan/vulkan.h>

namespace orbit_vulkan_layer {

// This class provides a wrapper for calls directly into the Vulkan loader.
// It is used, such that we can fake the called Vulkan functions in the tests.
//
// Note: in most cases we use the function pointers returned by "GetDevice/InstanceProcAddr",
// which directly point to the implementation in the next layer or ICD. So most used Vulkan
// functions don't need to show up here.
// See `DispatchTable` and `VulkanLayerController`.
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
