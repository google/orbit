// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LayerLogic.h"
#include "absl/base/casts.h"
#include "vulkan/vk_layer.h"
#include "vulkan/vulkan.h"

/*
 * The big picture:
 * This is the main entry point for Orbit's vulkan layer. The layer is structured as follows:
 * * All instrumented vulkan functions will hook into implementations found here
 *   (e.g. OrbitQueueSubmit)
 * * The actual logic of the layer is implemented in LayerLogic.h/.cpp. This has the following
 *    scheme: For each vk function, there is a PreCall*, Call*, and PostCall* function, where
 *    Call* will just forward the call to the "actual" vulkan function following the dispatch
 *    table (see DispatchTable).
 *  * There are the following helper classes to structure the actual layer logic:
 *     * CommandBufferManager.h: Which keeps track of command buffer allocations.
 *     * DispatchTable.h: Which provides virtual dispatch for the vulkan functions to be called.
 *     * QueryManager.h: Which keeps track of query pool slots e.g. used for timestamp queries
 *        and allows to assign those.
 *     * QueueManager keeps track of association of VkQueue(s) to devices.
 *
 *
 * For this free functions in this namespace:
 * As said, they act as entries to the layer.
 * OrbitGetDeviceProcAddr and OrbitGetInstanceProcAddr are the actual entry points, called by
 * the loader and potential other layers, and forward to all the functions that this layer
 * intercepts.
 *
 * The actual logic of the layer (and thus of each intercepted vulkan function) is implemented
 * in `LayerLogic`.
 *
 * Only the basic enumeration as well as the ProcAddr functions are implemented here.
 *
 */
namespace orbit_vulkan_layer {

#if defined(WIN32)
#define ORBIT_EXPORT extern "C" __declspec(dllexport) VK_LAYER_EXPORT
#else
#define ORBIT_EXPORT extern "C" VK_LAYER_EXPORT
#endif

// layer metadata
static constexpr const char* const kLayerName = "ORBIT_VK_LAYER";
static constexpr const char* const kLayerDescription =
    "Provides GPU insights for the Orbit System Profiler";

static constexpr const uint32_t kLayerImplVersion = 1;
static constexpr const uint32_t kLayerSpecVersion = VK_API_VERSION_1_1;

static LayerLogic logic_;

// ----------------------------------------------------------------------------
// Layer bootstrapping code
// ----------------------------------------------------------------------------

VKAPI_ATTR VkResult VKAPI_CALL OrbitCreateInstance(const VkInstanceCreateInfo* create_info,
                                                   const VkAllocationCallbacks* allocator,
                                                   VkInstance* instance) {
  VkResult result = logic_.PreCallAndCallCreateInstance(create_info, allocator, instance);
  logic_.PostCallCreateInstance(create_info, allocator, instance);
  return result;
}

VKAPI_ATTR void VKAPI_CALL OrbitDestroyInstance(VkInstance instance,
                                                const VkAllocationCallbacks* allocator) {
  logic_.CallDestroyInstance(instance, allocator);
  logic_.PostCallDestroyInstance(instance, allocator);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitCreateDevice(VkPhysicalDevice physical_device,
                                                 const VkDeviceCreateInfo* create_info,
                                                 const VkAllocationCallbacks* allocator,
                                                 VkDevice* device) {
  VkResult result =
      logic_.PreCallAndCallCreateDevice(physical_device, create_info, allocator, device);
  logic_.PostCallCreateDevice(physical_device, create_info, allocator, device);
  return result;
}

VKAPI_ATTR void VKAPI_CALL OrbitDestroyDevice(VkDevice device,
                                              const VkAllocationCallbacks* allocator) {
  logic_.CallDestroyDevice(device, allocator);
  logic_.PostCallDestroyDevice(device, allocator);
}

// ----------------------------------------------------------------------------
// Core layer logic
// ----------------------------------------------------------------------------

VKAPI_ATTR VkResult VKAPI_CALL OrbitCreateCommandPool(VkDevice device,
                                                      const VkCommandPoolCreateInfo* create_info,
                                                      const VkAllocationCallbacks* allocator,
                                                      VkCommandPool* command_pool) {
  VkResult result = logic_.CallCreateCommandPool(device, create_info, allocator, command_pool);
  logic_.PostCallCreateCommandPool(device, create_info, allocator, command_pool);
  return result;
}

VKAPI_ATTR void VKAPI_CALL OrbitDestroyCommandPool(VkDevice device, VkCommandPool command_pool,
                                                   const VkAllocationCallbacks* allocator) {
  logic_.CallDestroyCommandPool(device, command_pool, allocator);
  logic_.PostCallDestroyCommandPool(device, command_pool, allocator);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitResetCommandPool(VkDevice device, VkCommandPool command_pool,
                                                     VkCommandPoolResetFlags flags) {
  VkResult result = logic_.CallResetCommandPool(device, command_pool, flags);
  logic_.PostCallResetCommandPool(device, command_pool, flags);
  return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
OrbitAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* allocate_info,
                            VkCommandBuffer* command_buffers) {
  VkResult result = logic_.CallAllocateCommandBuffers(device, allocate_info, command_buffers);
  logic_.PostCallAllocateCommandBuffers(device, allocate_info, command_buffers);
  return result;
}

VKAPI_ATTR void VKAPI_CALL OrbitFreeCommandBuffers(VkDevice device, VkCommandPool command_pool,
                                                   uint32_t command_buffer_count,
                                                   const VkCommandBuffer* command_buffers) {
  logic_.CallFreeCommandBuffers(device, command_pool, command_buffer_count, command_buffers);
  logic_.PostCallFreeCommandBuffers(device, command_pool, command_buffer_count, command_buffers);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitBeginCommandBuffer(VkCommandBuffer command_buffer,
                                                       const VkCommandBufferBeginInfo* begin_info) {
  VkResult result = logic_.CallBeginCommandBuffer(command_buffer, begin_info);
  logic_.PostCallBeginCommandBuffer(command_buffer, begin_info);

  return result;
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitEndCommandBuffer(VkCommandBuffer command_buffer) {
  logic_.PreCallEndCommandBuffer(command_buffer);
  VkResult result = logic_.CallEndCommandBuffer(command_buffer);
  return result;
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitResetCommandBuffer(VkCommandBuffer command_buffer,
                                                       VkCommandBufferResetFlags flags) {
  logic_.PreCallResetCommandBuffer(command_buffer, flags);
  VkResult result = logic_.CallResetCommandBuffer(command_buffer, flags);

  return result;
}

VKAPI_ATTR void VKAPI_CALL OrbitGetDeviceQueue(VkDevice device, uint32_t queue_family_index,
                                               uint32_t queue_index, VkQueue* pQueue) {
  logic_.CallGetDeviceQueue(device, queue_family_index, queue_index, pQueue);
  logic_.PostCallGetDeviceQueue(device, queue_family_index, queue_index, pQueue);
}

VKAPI_ATTR void VKAPI_CALL OrbitGetDeviceQueue2(VkDevice device,
                                                const VkDeviceQueueInfo2* queue_info,
                                                VkQueue* queue) {
  logic_.CallGetDeviceQueue2(device, queue_info, queue);
  logic_.PostCallGetDeviceQueue2(device, queue_info, queue);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitQueueSubmit(VkQueue queue, uint32_t submit_count,
                                                const VkSubmitInfo* submits, VkFence fence) {
  VkResult result = logic_.CallQueueSubmit(queue, submit_count, submits, fence);
  logic_.PostCallQueueSubmit(queue, submit_count, submits, fence);
  return result;
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitQueuePresentKHR(VkQueue queue,
                                                    const VkPresentInfoKHR* present_info) {
  VkResult result = logic_.CallQueuePresentKHR(queue, present_info);
  logic_.PostCallQueuePresentKHR(queue, present_info);
  return result;
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitEnumeratePhysicalDevices(VkInstance instance,
                                                             uint32_t* physical_device_count,
                                                             VkPhysicalDevice* physical_devices) {
  VkResult result =
      logic_.CallEnumeratePhysicalDevices(instance, physical_device_count, physical_devices);
  logic_.PostCallEnumeratePhysicalDevices(instance, physical_device_count, physical_devices);
  return result;
}

// ----------------------------------------------------------------------------
// Layer enumeration functions
// ----------------------------------------------------------------------------
VKAPI_ATTR VkResult VKAPI_CALL
OrbitEnumerateInstanceLayerProperties(uint32_t* property_count, VkLayerProperties* properties) {
  // Vulkan spec dictates that we are only supposed to enumerate ourself
  if (property_count != nullptr) {
    *property_count = 1;
  }
  if (properties != nullptr) {
    snprintf(properties->layerName, strlen(kLayerName), kLayerName);
    snprintf(properties->description, strlen(kLayerDescription), kLayerDescription);
    properties->implementationVersion = kLayerImplVersion;
    properties->specVersion = kLayerSpecVersion;
  }

  return VK_SUCCESS;
}

// Deprecated by Khronos, but we'll support it in case older applications still
// use it.
VKAPI_ATTR VkResult VKAPI_CALL OrbitEnumerateDeviceLayerProperties(
    VkPhysicalDevice /*physical_device*/, uint32_t* property_count, VkLayerProperties* properties) {
  // This function is supposed to return the same results as
  // EnumerateInstanceLayerProperties since device layers were deprecated.
  return OrbitEnumerateInstanceLayerProperties(property_count, properties);
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitEnumerateInstanceExtensionProperties(
    const char* layer_name, uint32_t* property_count, VkExtensionProperties* /*properties*/) {
  // Inform the client that we have no extension properties if this layer
  // specifically is being queried.
  if (layer_name != nullptr && strcmp(layer_name, kLayerName) == 0) {
    if (property_count != nullptr) {
      *property_count = 0;
    }
    return VK_SUCCESS;
  }

  // Vulkan spec mandates returning this when this layer isn't being queried.
  return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL OrbitEnumerateDeviceExtensionProperties(
    VkPhysicalDevice physical_device, const char* layer_name, uint32_t* property_count,
    VkExtensionProperties* properties) {
  // Pass through any queries that aren't to us
  if (layer_name == nullptr || strcmp(layer_name, kLayerName) != 0) {
    if (physical_device == VK_NULL_HANDLE) {
      return VK_SUCCESS;
    }

    return logic_.CallEnumerateDeviceExtensionProperties(physical_device, layer_name,
                                                         property_count, properties);
  }

  // Don't expose any extensions
  if (property_count != nullptr) {
    *property_count = 0;
  }

  return VK_SUCCESS;
}

// ----------------------------------------------------------------------------
// GetProcAddr functions
// ----------------------------------------------------------------------------

#define ORBIT_GETPROCADDR(func)                              \
  if (strcmp(name, "vk" #func) == 0) {                       \
    return absl::bit_cast<PFN_vkVoidFunction>(&Orbit##func); \
  }

ORBIT_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL OrbitGetDeviceProcAddr(VkDevice device,
                                                                             const char* name) {
  // Functions available through GetInstanceProcAddr and GetDeviceProcAddr
  ORBIT_GETPROCADDR(GetDeviceProcAddr);
  ORBIT_GETPROCADDR(EnumerateDeviceLayerProperties);
  ORBIT_GETPROCADDR(EnumerateDeviceExtensionProperties);
  ORBIT_GETPROCADDR(CreateDevice);
  ORBIT_GETPROCADDR(DestroyDevice);

  ORBIT_GETPROCADDR(CreateCommandPool);
  ORBIT_GETPROCADDR(DestroyCommandPool);
  ORBIT_GETPROCADDR(ResetCommandPool);

  ORBIT_GETPROCADDR(AllocateCommandBuffers);
  ORBIT_GETPROCADDR(FreeCommandBuffers);

  ORBIT_GETPROCADDR(BeginCommandBuffer);
  ORBIT_GETPROCADDR(EndCommandBuffer);
  ORBIT_GETPROCADDR(ResetCommandBuffer);

  ORBIT_GETPROCADDR(QueueSubmit);
  ORBIT_GETPROCADDR(QueuePresentKHR);
  ORBIT_GETPROCADDR(GetDeviceQueue);
  ORBIT_GETPROCADDR(GetDeviceQueue2);

  return logic_.CallGetDeviceProcAddr(device, name);
}

ORBIT_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL OrbitGetInstanceProcAddr(VkInstance instance,
                                                                               const char* name) {
  // Functions available only through GetInstanceProcAddr
  ORBIT_GETPROCADDR(GetInstanceProcAddr);
  ORBIT_GETPROCADDR(CreateInstance);
  ORBIT_GETPROCADDR(DestroyInstance);
  ORBIT_GETPROCADDR(EnumerateInstanceLayerProperties);
  ORBIT_GETPROCADDR(EnumerateInstanceExtensionProperties);
  ORBIT_GETPROCADDR(EnumeratePhysicalDevices);

  // Functions available through GetInstanceProcAddr and GetDeviceProcAddr
  ORBIT_GETPROCADDR(GetDeviceProcAddr);
  ORBIT_GETPROCADDR(EnumerateDeviceLayerProperties);
  ORBIT_GETPROCADDR(EnumerateDeviceExtensionProperties);
  ORBIT_GETPROCADDR(CreateDevice);
  ORBIT_GETPROCADDR(DestroyDevice);

  ORBIT_GETPROCADDR(CreateCommandPool);
  ORBIT_GETPROCADDR(DestroyCommandPool);
  ORBIT_GETPROCADDR(ResetCommandPool);

  ORBIT_GETPROCADDR(AllocateCommandBuffers);
  ORBIT_GETPROCADDR(FreeCommandBuffers);

  ORBIT_GETPROCADDR(BeginCommandBuffer);
  ORBIT_GETPROCADDR(EndCommandBuffer);
  ORBIT_GETPROCADDR(ResetCommandBuffer);

  ORBIT_GETPROCADDR(QueueSubmit);
  ORBIT_GETPROCADDR(QueuePresentKHR);
  ORBIT_GETPROCADDR(GetDeviceQueue);
  ORBIT_GETPROCADDR(GetDeviceQueue2);

  return logic_.CallGetInstanceProcAddr(instance, name);
}

#undef ORBIT_GETPROCADDR

#undef ORBIT_EXPORT

}  // namespace orbit_vulkan_layer
