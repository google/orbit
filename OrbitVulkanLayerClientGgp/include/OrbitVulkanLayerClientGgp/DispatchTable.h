// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_CLIENT_GGP_DISPATCH_TABLE_H_
#define ORBIT_VULKAN_LAYER_CLIENT_GGP_DISPATCH_TABLE_H_

#include <vulkan/vulkan_core.h>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vk_layer_dispatch_table.h"
#include "vulkan/vulkan.h"

// Contains the logic related to the dispatch table so the creation of the table as well as the
// management of its keys are transparent to the main file.
class DispatchTable {
 public:
  void CreateInstanceDispatchTable(const VkInstance& instance,
                                   const PFN_vkGetInstanceProcAddr& get_instance_proc_addr);
  void CreateDeviceDispatchTable(const VkDevice& device,
                                 const PFN_vkGetDeviceProcAddr& get_device_proc_addr);
  void DestroyInstance(const VkInstance& instance);
  void DestroyDevice(const VkDevice& device);

  [[nodiscard]] PFN_vkVoidFunction CallGetDeviceProcAddr(VkDevice device, const char* name);
  [[nodiscard]] PFN_vkVoidFunction CallGetInstanceProcAddr(VkInstance instance, const char* name);

  [[nodiscard]] VkResult CallEnumerateDeviceExtensionProperties(
      const VkPhysicalDevice& physical_device, const char* layer_name, uint32_t* property_count,
      VkExtensionProperties* properties);

 private:
  absl::flat_hash_map<void*, VkLayerInstanceDispatchTable> instance_dispatch_;
  absl::flat_hash_map<void*, VkLayerDispatchTable> device_dispatch_;
};

#endif  // ORBIT_VULKAN_LAYER_CLIENT_GGP_DISPATCH_TABLE_H_