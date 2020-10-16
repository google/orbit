// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LAYER_LAYER_LOGIC_H_
#define ORBIT_LAYER_LAYER_LOGIC_H_

#include <OrbitBase/Logging.h>

#include "OrbitVulkanLayer/CommandBufferManager.h"
#include "OrbitVulkanLayer/DispatchTable.h"
#include "OrbitVulkanLayer/QueryManager.h"
#include "QueueManager.h"
#include "vulkan/vulkan.h"

namespace orbit::layer {

/**
 * This class controls the logic of this layer. For the instrumented vulkan functions,
 * it provides PreCall*, PostCall* and Call* functions, where the Call* function, just forward
 * to the next layer (using the dispatch table).
 * PreCall* functions are executed before the `actual` vulkan call and PostCall* afterwards.
 * PreCall/PostCall are omitted when not needed.
 *
 * Usage: For an instrumented vulkan function "X" a common pattern from the layers entry (Main.cpp)
 * would be:
 * ```
 * logic_.PreCallX(...);
 * logic_.CallX(...);
 * logic_.PostCallX(...);
 * ```
 */
class LayerLogic {
 public:
  LayerLogic() = default;
  ~LayerLogic() = default;

  [[nodiscard]] VkResult PreCallAndCallCreateInstance(const VkInstanceCreateInfo* create_info,
                                                      const VkAllocationCallbacks* allocator,
                                                      VkInstance* instance);
  void PostCallCreateInstance(const VkInstanceCreateInfo* create_info,
                              const VkAllocationCallbacks* allocator, VkInstance* instance);

  [[nodiscard]] PFN_vkVoidFunction CallGetDeviceProcAddr(VkDevice device, const char* name) {
    return dispatch_table_.GetDeviceProcAddr(device)(device, name);
  }

  [[nodiscard]] PFN_vkVoidFunction CallGetInstanceProcAddr(VkInstance instance, const char* name) {
    return dispatch_table_.GetInstanceProcAddr(instance)(instance, name);
  }

  void CallDestroyInstance(VkInstance instance, const VkAllocationCallbacks* allocator) {
    dispatch_table_.DestroyInstance(instance)(instance, allocator);
  }
  void PostCallDestroyInstance(VkInstance instance, const VkAllocationCallbacks* allocator);

  void CallDestroyDevice(VkDevice device, const VkAllocationCallbacks* allocator) {
    dispatch_table_.DestroyDevice(device)(device, allocator);
  }
  void PostCallDestroyDevice(VkDevice device, const VkAllocationCallbacks* allocator);

  [[nodiscard]] VkResult PreCallAndCallCreateDevice(VkPhysicalDevice physical_device,
                                                    const VkDeviceCreateInfo* create_info,
                                                    const VkAllocationCallbacks* allocator,
                                                    VkDevice* device);
  void PostCallCreateDevice(VkPhysicalDevice physical_device, const VkDeviceCreateInfo* create_info,
                            const VkAllocationCallbacks* allocator, VkDevice* device);

  [[nodiscard]] VkResult CallEnumerateDeviceExtensionProperties(VkPhysicalDevice physical_device,
                                                                const char* layer_name,
                                                                uint32_t* property_count,
                                                                VkExtensionProperties* properties) {
    CHECK(physical_device_to_instance_.contains(physical_device));
    const VkInstance& instance = physical_device_to_instance_.at(physical_device);
    return dispatch_table_.EnumerateDeviceExtensionProperties(instance)(physical_device, layer_name,
                                                                        property_count, properties);
  }

  [[nodiscard]] VkResult CallEnumeratePhysicalDevices(VkInstance instance,
                                                      uint32_t* physical_device_count,
                                                      VkPhysicalDevice* physical_devices) {
    return dispatch_table_.EnumeratePhysicalDevices(instance)(instance, physical_device_count,
                                                              physical_devices);
  }

  void PostCallEnumeratePhysicalDevices(VkInstance instance, uint32_t* physical_device_count,
                                        VkPhysicalDevice* physical_devices);

  [[nodiscard]] VkResult CallCreateCommandPool(VkDevice device,
                                               const VkCommandPoolCreateInfo* create_info,
                                               const VkAllocationCallbacks* allocator,
                                               VkCommandPool* command_pool) {
    return dispatch_table_.CreateCommandPool(device)(device, create_info, allocator, command_pool);
  }
  void PostCallCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* create_info,
                                 const VkAllocationCallbacks* allocator,
                                 VkCommandPool* command_pool);
  void CallDestroyCommandPool(VkDevice device, VkCommandPool command_pool,
                              const VkAllocationCallbacks* allocator) {
    dispatch_table_.DestroyCommandPool(device)(device, command_pool, allocator);
  }
  void PostCallDestroyCommandPool(VkDevice device, VkCommandPool command_pool,
                                  const VkAllocationCallbacks* allocator);
  [[nodiscard]] VkResult CallResetCommandPool(VkDevice device, VkCommandPool command_pool,
                                              VkCommandPoolResetFlags flags) {
    return dispatch_table_.ResetCommandPool(device)(device, command_pool, flags);
  }
  void PostCallResetCommandPool(VkDevice device, VkCommandPool command_pool,
                                VkCommandPoolResetFlags flags);
  [[nodiscard]] VkResult CallAllocateCommandBuffers(
      VkDevice device, const VkCommandBufferAllocateInfo* allocate_info,
      VkCommandBuffer* command_buffers) {
    return dispatch_table_.AllocateCommandBuffers(device)(device, allocate_info, command_buffers);
  }
  void PostCallAllocateCommandBuffers(VkDevice device,
                                      const VkCommandBufferAllocateInfo* allocate_info,
                                      VkCommandBuffer* command_buffers);

  void CallFreeCommandBuffers(VkDevice device, VkCommandPool command_pool,
                              uint32_t command_buffer_count,
                              const VkCommandBuffer* command_buffers) {
    return dispatch_table_.FreeCommandBuffers(device)(device, command_pool, command_buffer_count,
                                                      command_buffers);
  }
  void PostCallFreeCommandBuffers(VkDevice device, VkCommandPool command_pool,
                                  uint32_t command_buffer_count,
                                  const VkCommandBuffer* command_buffers);
  [[nodiscard]] VkResult CallBeginCommandBuffer(VkCommandBuffer command_buffer,
                                                const VkCommandBufferBeginInfo* begin_info) {
    const VkDevice& device = command_buffer_manager_.GetDeviceOfCommandBuffer(command_buffer);
    return dispatch_table_.BeginCommandBuffer(device)(command_buffer, begin_info);
  }
  void PostCallBeginCommandBuffer(VkCommandBuffer command_buffer,
                                  const VkCommandBufferBeginInfo* begin_info);

  void PreCallEndCommandBuffer(VkCommandBuffer command_buffer);
  [[nodiscard]] VkResult CallEndCommandBuffer(VkCommandBuffer command_buffer) {
    const VkDevice& device = command_buffer_manager_.GetDeviceOfCommandBuffer(command_buffer);
    return dispatch_table_.EndCommandBuffer(device)(command_buffer);
  }

  void PreCallResetCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferResetFlags flags);
  [[nodiscard]] VkResult CallResetCommandBuffer(VkCommandBuffer command_buffer,
                                                VkCommandBufferResetFlags flags) {
    const VkDevice& device = command_buffer_manager_.GetDeviceOfCommandBuffer(command_buffer);
    return dispatch_table_.ResetCommandBuffer(device)(command_buffer, flags);
  }

  void CallGetDeviceQueue(VkDevice device, uint32_t queue_family_index, uint32_t queue_index,
                          VkQueue* queue) {
    return dispatch_table_.GetDeviceQueue(device)(device, queue_family_index, queue_index, queue);
  }
  void PostCallGetDeviceQueue(VkDevice device, uint32_t queue_family_index, uint32_t queue_index,
                              VkQueue* queue);

  void CallGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* queue_info, VkQueue* queue) {
    return dispatch_table_.GetDeviceQueue2(device)(device, queue_info, queue);
  }
  void PostCallGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* queue_info,
                               VkQueue* queue);

  [[nodiscard]] VkResult CallQueueSubmit(VkQueue queue, uint32_t submit_count,
                                         const VkSubmitInfo* submits, VkFence fence) {
    const VkDevice& device = queue_manager_.GetDeviceOfQueue(queue);
    return dispatch_table_.QueueSubmit(device)(queue, submit_count, submits, fence);
  }
  void PostCallQueueSubmit(VkQueue queue, uint32_t submit_count, const VkSubmitInfo* submits,
                           VkFence fence);
  [[nodiscard]] VkResult CallQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info) {
    const VkDevice& device = queue_manager_.GetDeviceOfQueue(queue);
    return dispatch_table_.QueuePresentKHR(device)(queue, present_info);
  }
  void PostCallQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info);

 private:
  CommandBufferManager command_buffer_manager_;
  DispatchTable dispatch_table_;
  QueueManager queue_manager_;

  // TODO: Ideally move this to somewhere
  absl::Mutex mutex_;
  absl::flat_hash_map<VkPhysicalDevice, VkInstance> physical_device_to_instance_;
};

}  // namespace orbit::layer

#endif  // ORBIT_LAYER_LAYER_LOGIC_H_
