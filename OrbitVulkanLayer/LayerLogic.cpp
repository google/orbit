// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitVulkanLayer/LayerLogic.h"

#include <OrbitBase/Logging.h>

namespace orbit::layer {

VkResult LayerLogic::PreCallAndCallCreateInstance(const VkInstanceCreateInfo* create_info,
                                                  const VkAllocationCallbacks* allocator,
                                                  VkInstance* instance) {
  auto* layer_ci = absl::bit_cast<VkLayerInstanceCreateInfo*>(create_info->pNext);

  // Search through linked structs in pNext for link info.
  while (layer_ci != nullptr && (layer_ci->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO ||
                                 layer_ci->function != VK_LAYER_LINK_INFO)) {
    layer_ci = absl::bit_cast<VkLayerInstanceCreateInfo*>(layer_ci->pNext);
  }

  if (layer_ci == nullptr) {
    // No link info was found; we can't finish initializing
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr next_gipa = layer_ci->u.pLayerInfo->pfnNextGetInstanceProcAddr;

  // Advance linkage for next layer
  layer_ci->u.pLayerInfo = layer_ci->u.pLayerInfo->pNext;

  // Need to call vkCreateInstance down the chain to actually create the
  // instance.
  auto create_instance =
      absl::bit_cast<PFN_vkCreateInstance>(next_gipa(VK_NULL_HANDLE, "vkCreateInstance"));
  VkResult result = create_instance(create_info, allocator, instance);

  // Create dispatch table
  dispatch_table_.CreateInstanceDispatchTable(*instance, next_gipa);

  return result;
}

void LayerLogic::PostCallCreateInstance(const VkInstanceCreateInfo* /*create_info*/,
                                        const VkAllocationCallbacks* /*allocator*/,
                                        VkInstance* /*instance*/) {}

void LayerLogic::PostCallDestroyInstance(VkInstance instance,
                                         const VkAllocationCallbacks* /*allocator*/) {
  dispatch_table_.RemoveInstanceDispatchTable(instance);
}

void LayerLogic::PostCallDestroyDevice(VkDevice device,
                                       const VkAllocationCallbacks* /*allocator*/) {
  dispatch_table_.RemoveDeviceDispatchTable(device);
}

VkResult LayerLogic::PreCallAndCallCreateDevice(
    VkPhysicalDevice /*physical_device*/ physical_device, const VkDeviceCreateInfo* create_info,
    const VkAllocationCallbacks* allocator /*allocator*/, VkDevice* device) {
  auto* layer_ci = absl::bit_cast<VkLayerDeviceCreateInfo*>(create_info->pNext);

  // Search through linked structs in pNext for link info.
  while (layer_ci != nullptr && (layer_ci->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO ||
                                 layer_ci->function != VK_LAYER_LINK_INFO)) {
    layer_ci = absl::bit_cast<VkLayerDeviceCreateInfo*>(layer_ci->pNext);
  }

  if (layer_ci == nullptr) {
    // No link info was found; we can't finish initializing
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr next_gipa = layer_ci->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  PFN_vkGetDeviceProcAddr next_gdpa = layer_ci->u.pLayerInfo->pfnNextGetDeviceProcAddr;

  // Advance linkage for next layer
  layer_ci->u.pLayerInfo = layer_ci->u.pLayerInfo->pNext;

  // Need to call vkCreateDevice down the chain to actually create the device
  auto create_device_function =
      absl::bit_cast<PFN_vkCreateDevice>(next_gipa(VK_NULL_HANDLE, "vkCreateDevice"));
  VkResult result = create_device_function(physical_device, create_info, allocator, device);

  // Create dispatch table
  dispatch_table_.CreateDeviceDispatchTable(*device, next_gdpa);

  return result;
}

void LayerLogic::PostCallCreateDevice(VkPhysicalDevice /*physical_device*/,
                                      const VkDeviceCreateInfo* /*create_info*/,
                                      const VkAllocationCallbacks* /*allocator*/,
                                      VkDevice* /*device*/) {}

void LayerLogic::PostCallCreateCommandPool(VkDevice /*device*/,
                                           const VkCommandPoolCreateInfo* /*create_info*/,
                                           const VkAllocationCallbacks* /*allocator*/,
                                           VkCommandPool* command_pool) {
  command_buffer_manager_.TrackCommandPool(*command_pool);
}

void LayerLogic::PostCallDestroyCommandPool(VkDevice /*device*/, VkCommandPool command_pool,
                                            const VkAllocationCallbacks* /*allocator*/) {
  command_buffer_manager_.UnTrackCommandPool(command_pool);
}

void LayerLogic::PostCallResetCommandPool(VkDevice /*device*/, VkCommandPool /*command_pool*/,
                                          VkCommandPoolResetFlags /*flags*/) {}

void LayerLogic::PostCallAllocateCommandBuffers(VkDevice device,
                                                const VkCommandBufferAllocateInfo* allocate_info,
                                                VkCommandBuffer* command_buffers) {
  const VkCommandPool& pool = allocate_info->commandPool;
  const uint32_t cb_count = allocate_info->commandBufferCount;
  command_buffer_manager_.TrackCommandBuffers(device, pool, command_buffers, cb_count);
}

void LayerLogic::PostCallFreeCommandBuffers(VkDevice device, VkCommandPool command_pool,
                                            uint32_t command_buffer_count,
                                            const VkCommandBuffer* command_buffers) {
  command_buffer_manager_.UnTrackCommandBuffers(device, command_pool, command_buffers,
                                                command_buffer_count);
}

void LayerLogic::PostCallBeginCommandBuffer(VkCommandBuffer /*command_buffer*/,
                                            const VkCommandBufferBeginInfo* /*begin_info*/) {}

void LayerLogic::PreCallEndCommandBuffer(VkCommandBuffer /*command_buffer*/) {}

void LayerLogic::PreCallResetCommandBuffer(VkCommandBuffer /*command_buffer*/,
                                           VkCommandBufferResetFlags /*flags*/) {}

void LayerLogic::PostCallQueueSubmit(VkQueue /*queue*/, uint32_t /*submit_count*/,
                                     const VkSubmitInfo* /*submits*/, VkFence /*fence*/) {}

void LayerLogic::PostCallQueuePresentKHR(VkQueue /*queue*/,
                                         const VkPresentInfoKHR* /*present_info*/) {}
void LayerLogic::PostCallGetDeviceQueue(VkDevice device, uint32_t /*queue_family_index*/,
                                        uint32_t /*queue_index*/, VkQueue* queue) {
  queue_manager_.TrackQueue(*queue, device);
}
void LayerLogic::PostCallGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* /*queue_info*/,
                                         VkQueue* queue) {
  queue_manager_.TrackQueue(*queue, device);
}

void LayerLogic::PostCallEnumeratePhysicalDevices(VkInstance instance,
                                                  uint32_t* physical_device_count,
                                                  VkPhysicalDevice* physical_devices) {
  if (physical_device_count != nullptr && physical_devices != nullptr) {
    // Map these devices to this instance so that we can map each physical
    // device back to a dispatch table when handling
    // EnumerateDeviceExtensionProperties. Note that this is hardly error-proof,
    // but it's impossible to handle this perfectly since physical devices may
    // be in use by multiple instances.
    {
      absl::WriterMutexLock lock(&mutex_);
      for (uint32_t i = 0; i < *physical_device_count; ++i) {
        const VkPhysicalDevice& device = physical_devices[i];
        physical_device_to_instance_[device] = instance;
      }
    }
  }
}

}  // namespace orbit::layer