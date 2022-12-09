// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <vector>

#include "DeviceManager.h"
#include "DispatchTable.h"
#include "QueueManager.h"
#include "SubmissionTracker.h"
#include "TimerQueryPool.h"
#include "VulkanLayerController.h"
#include "VulkanWrapper.h"

/*
 * The big picture:
 * This is the main entry point for Orbit's Vulkan layer. The layer is structured as follows:
 * - All instrumented Vulkan functions will hook into implementations found here
 *   (e.g. OrbitQueueSubmit) which will delegate to the `VulkanLayerController`.
 * - For every `vkX` function, this controller has an `OnX` functions that will perform the actual
 *   Vulkan call (using DispatchTable), but also glue together the logic of the layer.
 * - These are the helper classes to structure the actual layer logic:
 * -- SubmissionTracker: Which is the heart of the layer logic. It keeps track of
 *     command buffer usages and timings, debug markers and submissions.
 * -- DispatchTable: It provides virtual dispatch for the Vulkan functions to be called.
 * -- TimerQueryPool: Which keeps track of query pool slots used for timestamp queries and allows
 *     to assign those.
 * -- VulkanLayerProducer: This is the producer used for the IPC with Orbit. Results will be sent
 *     by this as CaptureEvent protos.
 * -- DeviceManager: It tracks the association of a VkDevice to the VkPhysicalDevice.
 * -- QueueManager: It keeps track of association of VkQueue(s) to devices.
 *
 * The free functions in this namespace act as entry points to the layer.
 * OrbitGetDeviceProcAddr and OrbitGetInstanceProcAddr are the actual entry points, called by
 * the loader and potential other layers. They return pointers to the functions that this layer
 * intercepts. All other functions are accessible via those two lookup functions.
 */
namespace orbit_vulkan_layer {

using DeviceMangerImpl = DeviceManager<DispatchTable>;
using TimerQueryPoolImpl = TimerQueryPool<DispatchTable>;
using SubmissionTrackerImpl =
    SubmissionTracker<DispatchTable, DeviceMangerImpl, TimerQueryPoolImpl>;
static VulkanLayerController<DispatchTable, QueueManager, DeviceMangerImpl, TimerQueryPoolImpl,
                             SubmissionTrackerImpl, VulkanWrapper>
    controller;

// ----------------------------------------------------------------------------
// Layer bootstrapping code
// ----------------------------------------------------------------------------

VKAPI_ATTR VkResult VKAPI_CALL OrbitCreateInstance(const VkInstanceCreateInfo* create_info,
                                                   const VkAllocationCallbacks* allocator,
                                                   VkInstance* instance) {
  return controller.OnCreateInstance(create_info, allocator, instance);
}

VKAPI_ATTR void VKAPI_CALL OrbitDestroyInstance(VkInstance instance,
                                                const VkAllocationCallbacks* allocator) {
  controller.OnDestroyInstance(instance, allocator);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitCreateDevice(VkPhysicalDevice physical_device,
                                                 const VkDeviceCreateInfo* create_info,
                                                 const VkAllocationCallbacks* allocator,
                                                 VkDevice* device) {
  return controller.OnCreateDevice(physical_device, create_info, allocator, device);
}

VKAPI_ATTR void VKAPI_CALL OrbitDestroyDevice(VkDevice device,
                                              const VkAllocationCallbacks* allocator) {
  controller.OnDestroyDevice(device, allocator);
}

// ----------------------------------------------------------------------------
// Core layer logic
// ----------------------------------------------------------------------------

VKAPI_ATTR VkResult VKAPI_CALL OrbitResetCommandPool(VkDevice device, VkCommandPool command_pool,
                                                     VkCommandPoolResetFlags flags) {
  return controller.OnResetCommandPool(device, command_pool, flags);
}

VKAPI_ATTR VkResult VKAPI_CALL
OrbitAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* allocate_info,
                            VkCommandBuffer* command_buffers) {
  return controller.OnAllocateCommandBuffers(device, allocate_info, command_buffers);
}

VKAPI_ATTR void VKAPI_CALL OrbitFreeCommandBuffers(VkDevice device, VkCommandPool command_pool,
                                                   uint32_t command_buffer_count,
                                                   const VkCommandBuffer* command_buffers) {
  controller.OnFreeCommandBuffers(device, command_pool, command_buffer_count, command_buffers);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitBeginCommandBuffer(VkCommandBuffer command_buffer,
                                                       const VkCommandBufferBeginInfo* begin_info) {
  return controller.OnBeginCommandBuffer(command_buffer, begin_info);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitEndCommandBuffer(VkCommandBuffer command_buffer) {
  return controller.OnEndCommandBuffer(command_buffer);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitResetCommandBuffer(VkCommandBuffer command_buffer,
                                                       VkCommandBufferResetFlags flags) {
  return controller.OnResetCommandBuffer(command_buffer, flags);
}

VKAPI_ATTR void VKAPI_CALL OrbitGetDeviceQueue(VkDevice device, uint32_t queue_family_index,
                                               uint32_t queue_index, VkQueue* p_queue) {
  controller.OnGetDeviceQueue(device, queue_family_index, queue_index, p_queue);
}

VKAPI_ATTR void VKAPI_CALL OrbitGetDeviceQueue2(VkDevice device,
                                                const VkDeviceQueueInfo2* queue_info,
                                                VkQueue* queue) {
  controller.OnGetDeviceQueue2(device, queue_info, queue);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitQueueSubmit(VkQueue queue, uint32_t submit_count,
                                                const VkSubmitInfo* submits, VkFence fence) {
  return controller.OnQueueSubmit(queue, submit_count, submits, fence);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitQueuePresentKHR(VkQueue queue,
                                                    const VkPresentInfoKHR* present_info) {
  return controller.OnQueuePresentKHR(queue, present_info);
}

// ----------------------------------------------------------------------------
// Implemented and used extension methods
// ----------------------------------------------------------------------------

VKAPI_ATTR void VKAPI_CALL OrbitCmdBeginDebugUtilsLabelEXT(VkCommandBuffer command_buffer,
                                                           const VkDebugUtilsLabelEXT* label_info) {
  controller.OnCmdBeginDebugUtilsLabelEXT(command_buffer, label_info);
}

VKAPI_ATTR void VKAPI_CALL OrbitCmdEndDebugUtilsLabelEXT(VkCommandBuffer command_buffer) {
  controller.OnCmdEndDebugUtilsLabelEXT(command_buffer);
}

VKAPI_ATTR void VKAPI_CALL OrbitCmdDebugMarkerBeginEXT(
    VkCommandBuffer command_buffer, const VkDebugMarkerMarkerInfoEXT* marker_info) {
  controller.OnCmdDebugMarkerBeginEXT(command_buffer, marker_info);
}

VKAPI_ATTR void VKAPI_CALL OrbitCmdDebugMarkerEndEXT(VkCommandBuffer command_buffer) {
  controller.OnCmdDebugMarkerEndEXT(command_buffer);
}

// ----------------------------------------------------------------------------
// Unused but implemented extension methods (need to implement all methods of
// an extension)
// ----------------------------------------------------------------------------
VKAPI_ATTR void VKAPI_CALL OrbitCmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer command_buffer, const VkDebugUtilsLabelEXT* label_info) {
  controller.OnCmdInsertDebugUtilsLabelEXT(command_buffer, label_info);
}

VKAPI_ATTR void VKAPI_CALL OrbitCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* messenger) {
  controller.OnCreateDebugUtilsMessengerEXT(instance, create_info, allocator, messenger);
}

VKAPI_ATTR void VKAPI_CALL
OrbitDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks* allocator) {
  controller.OnDestroyDebugUtilsMessengerEXT(instance, messenger, allocator);
}

VKAPI_ATTR void VKAPI_CALL
OrbitQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* label_info) {
  controller.OnQueueBeginDebugUtilsLabelEXT(queue, label_info);
}

VKAPI_ATTR void VKAPI_CALL OrbitQueueEndDebugUtilsLabelEXT(VkQueue queue) {
  controller.OnQueueEndDebugUtilsLabelEXT(queue);
}

VKAPI_ATTR void VKAPI_CALL
OrbitQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* label_info) {
  controller.OnQueueInsertDebugUtilsLabelEXT(queue, label_info);
}

VKAPI_ATTR VkResult VKAPI_CALL
OrbitSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* name_info) {
  return controller.OnSetDebugUtilsObjectNameEXT(device, name_info);
}

VKAPI_ATTR VkResult VKAPI_CALL
OrbitSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* tag_info) {
  return controller.OnSetDebugUtilsObjectTagEXT(device, tag_info);
}

VKAPI_ATTR void VKAPI_CALL OrbitSubmitDebugUtilsMessageEXT(
    VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data) {
  controller.OnSubmitDebugUtilsMessageEXT(instance, message_severity, message_types, callback_data);
}

VKAPI_ATTR void VKAPI_CALL OrbitCmdDebugMarkerInsertEXT(
    VkCommandBuffer command_buffer, const VkDebugMarkerMarkerInfoEXT* marker_info) {
  controller.OnCmdDebugMarkerInsertEXT(command_buffer, marker_info);
}

VKAPI_ATTR VkResult VKAPI_CALL
OrbitDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* name_info) {
  return controller.OnDebugMarkerSetObjectNameEXT(device, name_info);
}

VKAPI_ATTR VkResult VKAPI_CALL
OrbitDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* tag_info) {
  return controller.OnDebugMarkerSetObjectTagEXT(device, tag_info);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitCreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator, VkDebugReportCallbackEXT* callback) {
  return controller.OnCreateDebugReportCallbackEXT(instance, create_info, allocator, callback);
}

VKAPI_ATTR void VKAPI_CALL
OrbitDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                           VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location,
                           int32_t message_code, const char* layer_prefix, const char* message) {
  controller.OnDebugReportMessageEXT(instance, flags, object_type, object, location, message_code,
                                     layer_prefix, message);
}

VKAPI_ATTR void VKAPI_CALL
OrbitDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                   const VkAllocationCallbacks* allocator) {
  controller.OnDestroyDebugReportCallbackEXT(instance, callback, allocator);
}
// ----------------------------------------------------------------------------
// Layer enumeration functions
// ----------------------------------------------------------------------------

VKAPI_ATTR VkResult VKAPI_CALL
OrbitEnumerateInstanceLayerProperties(uint32_t* property_count, VkLayerProperties* properties) {
  return controller.OnEnumerateInstanceLayerProperties(property_count, properties);
}

// Deprecated by Khronos, but we'll support it in case older applications still use it.
VKAPI_ATTR VkResult VKAPI_CALL OrbitEnumerateDeviceLayerProperties(
    VkPhysicalDevice /*physical_device*/, uint32_t* property_count, VkLayerProperties* properties) {
  // This function is supposed to return the same results as
  // EnumerateInstanceLayerProperties since device layers were deprecated.
  return controller.OnEnumerateInstanceLayerProperties(property_count, properties);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitEnumerateInstanceExtensionProperties(
    const char* layer_name, uint32_t* property_count, VkExtensionProperties* properties) {
  return controller.OnEnumerateInstanceExtensionProperties(layer_name, property_count, properties);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitEnumerateDeviceExtensionProperties(
    VkPhysicalDevice physical_device, const char* layer_name, uint32_t* property_count,
    VkExtensionProperties* properties) {
  return controller.OnEnumerateDeviceExtensionProperties(physical_device, layer_name,
                                                         property_count, properties);
}

// ----------------------------------------------------------------------------
// GetProcAddr functions
// ----------------------------------------------------------------------------

#define RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(func)     \
  do {                                                         \
    if (strcmp(name, "vk" #func) == 0) {                       \
      return absl::bit_cast<PFN_vkVoidFunction>(&Orbit##func); \
    }                                                          \
  } while (false)

extern "C" VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
OrbitGetDeviceProcAddr(VkDevice device, const char* name) {
  // Functions available through GetInstanceProcAddr and GetDeviceProcAddr
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(GetDeviceProcAddr);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EnumerateDeviceLayerProperties);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EnumerateDeviceExtensionProperties);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CreateDevice);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DestroyDevice);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(ResetCommandPool);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(AllocateCommandBuffers);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(FreeCommandBuffers);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(BeginCommandBuffer);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EndCommandBuffer);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(ResetCommandBuffer);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueSubmit);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueuePresentKHR);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(GetDeviceQueue);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(GetDeviceQueue2);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdBeginDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdEndDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdDebugMarkerBeginEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdDebugMarkerEndEXT);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(SetDebugUtilsObjectNameEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(SetDebugUtilsObjectTagEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueBeginDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueEndDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueInsertDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdInsertDebugUtilsLabelEXT);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DebugMarkerSetObjectTagEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DebugMarkerSetObjectNameEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdDebugMarkerInsertEXT);

  return controller.ForwardGetDeviceProcAddr(device, name);
}

extern "C" VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
OrbitGetInstanceProcAddr(VkInstance instance, const char* name) {
  // Functions available only through GetInstanceProcAddr
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(GetInstanceProcAddr);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CreateInstance);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DestroyInstance);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EnumerateInstanceLayerProperties);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EnumerateInstanceExtensionProperties);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CreateDebugUtilsMessengerEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DestroyDebugUtilsMessengerEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(SubmitDebugUtilsMessageEXT);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CreateDebugReportCallbackEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DestroyDebugReportCallbackEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DebugReportMessageEXT);

  // Functions available through GetInstanceProcAddr and GetDeviceProcAddr
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(GetDeviceProcAddr);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EnumerateDeviceLayerProperties);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EnumerateDeviceExtensionProperties);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CreateDevice);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DestroyDevice);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(ResetCommandPool);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(AllocateCommandBuffers);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(FreeCommandBuffers);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(BeginCommandBuffer);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(EndCommandBuffer);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(ResetCommandBuffer);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueSubmit);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueuePresentKHR);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(GetDeviceQueue);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(GetDeviceQueue2);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdBeginDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdEndDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdDebugMarkerBeginEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdDebugMarkerEndEXT);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(SetDebugUtilsObjectNameEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(SetDebugUtilsObjectTagEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueBeginDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueEndDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(QueueInsertDebugUtilsLabelEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdInsertDebugUtilsLabelEXT);

  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DebugMarkerSetObjectTagEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(DebugMarkerSetObjectNameEXT);
  RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION(CmdDebugMarkerInsertEXT);

  return controller.ForwardGetInstanceProcAddr(instance, name);
}

#undef RETURN_ORBIT_FUNCTION_IF_MATCHES_VK_FUNCTION

}  // namespace orbit_vulkan_layer
