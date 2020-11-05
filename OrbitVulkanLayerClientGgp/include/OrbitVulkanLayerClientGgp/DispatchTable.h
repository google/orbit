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

class DispatchTable {
 public:
  void CreateInstanceDispatchTable(const VkInstance& instance,
                                   const PFN_vkGetInstanceProcAddr& gpa);
  void CreateDeviceDispatchTable(const VkDevice& device, const PFN_vkGetDeviceProcAddr& gdpa);
  void DestroyInstance(const VkInstance& instance);
  void DestroyDevice(const VkDevice& device);

  [[nodiscard]] PFN_vkVoidFunction CallGetDeviceProcAddr(VkDevice device, const char* name);
  [[nodiscard]] PFN_vkVoidFunction CallGetInstanceProcAddr(VkInstance instance, const char* name);

  [[nodiscard]] VkResult CallEnumerateDeviceExtensionProperties(
      const VkPhysicalDevice& physical_device, const char* layer_name, uint32_t* property_count,
      VkExtensionProperties* properties);

  [[nodiscard]] VkResult CallBeginCommandBuffer(VkCommandBuffer command_buffer,
                                                const VkCommandBufferBeginInfo* begin_info);

  void CallCmdDraw(VkCommandBuffer command_buffer, uint32_t vertex_count, uint32_t instance_count,
                   uint32_t first_vertex, uint32_t first_instance);

  void CallCmdDrawIndexed(VkCommandBuffer command_buffer, uint32_t index_count,
                          uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
                          uint32_t first_instance);

  [[nodiscard]] VkResult CallEndCommandBuffer(VkCommandBuffer command_buffer);

 private:
  absl::flat_hash_map<void*, VkLayerInstanceDispatchTable> instance_dispatch_;
  absl::flat_hash_map<void*, VkLayerDispatchTable> device_dispatch_;
};

#endif  // ORBIT_VULKAN_LAYER_CLIENT_GGP_DISPATCH_TABLE_H_