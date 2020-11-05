// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitVulkanLayerClientGgp/DispatchTable.h"

#include <string.h>

#include "OrbitBase/Logging.h"
#include "absl/base/casts.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vk_layer.h"
#include "vulkan/vk_layer_dispatch_table.h"
#include "vulkan/vulkan.h"

namespace {
// use the loader's dispatch table pointer as a key for dispatch map lookups
template <typename DispatchableType>
void* GetKey(DispatchableType inst) {
  return *absl::bit_cast<void**>(inst);
}
}  // namespace

void DispatchTable::CreateInstanceDispatchTable(const VkInstance& instance,
                                                const PFN_vkGetInstanceProcAddr& gpa) {
  // fetch our own dispatch table for the functions we need, into the next layer
  VkLayerInstanceDispatchTable dispatch_table;
  dispatch_table.GetInstanceProcAddr =
      absl::bit_cast<PFN_vkGetInstanceProcAddr>(gpa(instance, "vkGetInstanceProcAddr"));
  dispatch_table.DestroyInstance =
      absl::bit_cast<PFN_vkDestroyInstance>(gpa(instance, "vkDestroyInstance"));
  dispatch_table.EnumerateDeviceExtensionProperties =
      absl::bit_cast<PFN_vkEnumerateDeviceExtensionProperties>(
          gpa(instance, "vkEnumerateDeviceExtensionProperties"));

  // store the table by key
  instance_dispatch_[GetKey(instance)] = dispatch_table;
}

void DispatchTable::CreateDeviceDispatchTable(const VkDevice& device,
                                              const PFN_vkGetDeviceProcAddr& gdpa) {
  // fetch our own dispatch table for the functions we need, into the next layer
  VkLayerDispatchTable dispatch_table;
  dispatch_table.GetDeviceProcAddr =
      absl::bit_cast<PFN_vkGetDeviceProcAddr>(gdpa(device, "vkGetDeviceProcAddr"));
  dispatch_table.DestroyDevice =
      absl::bit_cast<PFN_vkDestroyDevice>(gdpa(device, "vkDestroyDevice"));
  dispatch_table.BeginCommandBuffer =
      absl::bit_cast<PFN_vkBeginCommandBuffer>(gdpa(device, "vkBeginCommandBuffer"));
  dispatch_table.CmdDraw = absl::bit_cast<PFN_vkCmdDraw>(gdpa(device, "vkCmdDraw"));
  dispatch_table.CmdDrawIndexed =
      absl::bit_cast<PFN_vkCmdDrawIndexed>(gdpa(device, "vkCmdDrawIndexed"));
  dispatch_table.EndCommandBuffer =
      absl::bit_cast<PFN_vkEndCommandBuffer>(gdpa(device, "vkEndCommandBuffer"));

  // store the table by key
  device_dispatch_[GetKey(device)] = dispatch_table;
}

void DispatchTable::DestroyInstance(const VkInstance& instance) {
  instance_dispatch_.erase(GetKey(instance));
}

void DispatchTable::DestroyDevice(const VkDevice& device) {
  device_dispatch_.erase(GetKey(device));
}

PFN_vkVoidFunction DispatchTable::CallGetDeviceProcAddr(VkDevice device, const char* name) {
  return device_dispatch_[GetKey(device)].GetDeviceProcAddr(device, name);
}

PFN_vkVoidFunction DispatchTable::CallGetInstanceProcAddr(VkInstance instance, const char* name) {
  return instance_dispatch_[GetKey(instance)].GetInstanceProcAddr(instance, name);
}

VkResult DispatchTable::CallEnumerateDeviceExtensionProperties(
    const VkPhysicalDevice& physical_device, const char* layer_name, uint32_t* property_count,
    VkExtensionProperties* properties) {
  return instance_dispatch_[GetKey(physical_device)].EnumerateDeviceExtensionProperties(
      physical_device, layer_name, property_count, properties);
}

VkResult DispatchTable::CallBeginCommandBuffer(VkCommandBuffer command_buffer,
                                               const VkCommandBufferBeginInfo* begin_info) {
  return device_dispatch_[GetKey(command_buffer)].BeginCommandBuffer(command_buffer, begin_info);
}

void DispatchTable::CallCmdDraw(VkCommandBuffer command_buffer, uint32_t vertex_count,
                                uint32_t instance_count, uint32_t first_vertex,
                                uint32_t first_instance) {
  device_dispatch_[GetKey(command_buffer)].CmdDraw(command_buffer, vertex_count, instance_count,
                                                   first_vertex, first_instance);
}

void DispatchTable::CallCmdDrawIndexed(VkCommandBuffer command_buffer, uint32_t index_count,
                                       uint32_t instance_count, uint32_t first_index,
                                       int32_t vertex_offset, uint32_t first_instance) {
  device_dispatch_[GetKey(command_buffer)].CmdDrawIndexed(
      command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

VkResult DispatchTable::CallEndCommandBuffer(VkCommandBuffer command_buffer) {
  return device_dispatch_[GetKey(command_buffer)].EndCommandBuffer(command_buffer);
}
