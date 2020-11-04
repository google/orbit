// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <string.h>

#include "OrbitBase/Logging.h"
#include "absl/base/casts.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vk_layer.h"
#include "vulkan/vk_layer_dispatch_table.h"
#include "vulkan/vulkan.h"

#undef VK_LAYER_EXPORT
#if defined(WIN32)
#define VK_LAYER_EXPORT extern "C" __declspec(dllexport)
#else
#define VK_LAYER_EXPORT extern "C"
#endif

// Layer information
static constexpr char const* kLayerName = "VK_LAYER_ORBIT_CAPTURE_CLIENT";
static constexpr char const* kLayerDescription =
    "Layer that contains Orbit Client implementation to run captures";
static constexpr uint32_t const kLayerImplementationVersion = 1;
static constexpr uint32_t const kLayerSpecVersion = VK_API_VERSION_1_1;

absl::Mutex mutex;

// use the loader's dispatch table pointer as a key for dispatch map lookups
template <typename DispatchableType>
void* GetKey(DispatchableType inst) {
  return *absl::bit_cast<void**>(inst);
}

// layer book-keeping information, to store dispatch tables by key
absl::flat_hash_map<void*, VkLayerInstanceDispatchTable> instance_dispatch;
absl::flat_hash_map<void*, VkLayerDispatchTable> device_dispatch;

// actual data we're recording in this layer
struct CommandStats {
  uint32_t draw_count = 0, instance_count = 0, vert_count = 0;
};

std::map<VkCommandBuffer, CommandStats> command_buffer_stats;

// --------------------------------------------------------------------------------
// Layer init and shutdown
// --------------------------------------------------------------------------------

VK_LAYER_EXPORT VkResult VKAPI_CALL
OrbitCaptureClientCreateInstance(const VkInstanceCreateInfo* instance_create_info,
                                 const VkAllocationCallbacks* allocator, VkInstance* instance) {
  auto* layer_instance_create_info =
      absl::bit_cast<VkLayerInstanceCreateInfo*>(instance_create_info->pNext);

  // step through the chain of pNext until we get to the link info
  while (layer_instance_create_info != nullptr &&
         (layer_instance_create_info->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO ||
          layer_instance_create_info->function != VK_LAYER_LINK_INFO)) {
    layer_instance_create_info =
        absl::bit_cast<VkLayerInstanceCreateInfo*>(layer_instance_create_info->pNext);
  }

  if (layer_instance_create_info == nullptr) {
    // No loader instance create info
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr gpa =
      layer_instance_create_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  // move chain on for next layer
  layer_instance_create_info->u.pLayerInfo = layer_instance_create_info->u.pLayerInfo->pNext;

  auto create_instance =
      absl::bit_cast<PFN_vkCreateInstance>(gpa(VK_NULL_HANDLE, "vkCreateInstance"));

  VkResult result = create_instance(instance_create_info, allocator, instance);

  // fetch our own dispatch table for the functions we need, into the next layer
  VkLayerInstanceDispatchTable dispatch_table;
  dispatch_table.GetInstanceProcAddr =
      absl::bit_cast<PFN_vkGetInstanceProcAddr>(gpa(*instance, "vkGetInstanceProcAddr"));
  dispatch_table.DestroyInstance =
      absl::bit_cast<PFN_vkDestroyInstance>(gpa(*instance, "vkDestroyInstance"));
  dispatch_table.EnumerateDeviceExtensionProperties =
      absl::bit_cast<PFN_vkEnumerateDeviceExtensionProperties>(
          gpa(*instance, "vkEnumerateDeviceExtensionProperties"));

  // store the table by key
  {
    absl::WriterMutexLock lock(&mutex);
    instance_dispatch[GetKey(*instance)] = dispatch_table;
  }

  return result;
}

VK_LAYER_EXPORT void VKAPI_CALL
OrbitCaptureClientDestroyInstance(VkInstance instance, const VkAllocationCallbacks* /*allocator*/) {
  absl::WriterMutexLock lock(&mutex);
  instance_dispatch.erase(GetKey(instance));
}

VK_LAYER_EXPORT VkResult VKAPI_CALL OrbitCaptureClientCreateDevice(
    VkPhysicalDevice physical_device, const VkDeviceCreateInfo* device_create_info,
    const VkAllocationCallbacks* allocator, VkDevice* device) {
  auto* layer_device_create_info =
      absl::bit_cast<VkLayerDeviceCreateInfo*>(device_create_info->pNext);

  // step through the chain of pNext until we get to the link info
  while (layer_device_create_info != nullptr &&
         (layer_device_create_info->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO ||
          layer_device_create_info->function != VK_LAYER_LINK_INFO)) {
    layer_device_create_info =
        absl::bit_cast<VkLayerDeviceCreateInfo*>(layer_device_create_info->pNext);
  }

  if (layer_device_create_info == nullptr) {
    // No loader instance create info
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr gipa =
      layer_device_create_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  PFN_vkGetDeviceProcAddr gdpa = layer_device_create_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
  // move chain on for next layer
  layer_device_create_info->u.pLayerInfo = layer_device_create_info->u.pLayerInfo->pNext;

  auto create_device = absl::bit_cast<PFN_vkCreateDevice>(gipa(VK_NULL_HANDLE, "vkCreateDevice"));

  VkResult result = create_device(physical_device, device_create_info, allocator, device);

  // fetch our own dispatch table for the functions we need, into the next layer
  VkLayerDispatchTable dispatch_table;
  dispatch_table.GetDeviceProcAddr =
      absl::bit_cast<PFN_vkGetDeviceProcAddr>(gdpa(*device, "vkGetDeviceProcAddr"));
  dispatch_table.DestroyDevice =
      absl::bit_cast<PFN_vkDestroyDevice>(gdpa(*device, "vkDestroyDevice"));
  dispatch_table.BeginCommandBuffer =
      absl::bit_cast<PFN_vkBeginCommandBuffer>(gdpa(*device, "vkBeginCommandBuffer"));
  dispatch_table.CmdDraw = absl::bit_cast<PFN_vkCmdDraw>(gdpa(*device, "vkCmdDraw"));
  dispatch_table.CmdDrawIndexed =
      absl::bit_cast<PFN_vkCmdDrawIndexed>(gdpa(*device, "vkCmdDrawIndexed"));
  dispatch_table.EndCommandBuffer =
      absl::bit_cast<PFN_vkEndCommandBuffer>(gdpa(*device, "vkEndCommandBuffer"));

  // store the table by key
  {
    absl::WriterMutexLock lock(&mutex);
    device_dispatch[GetKey(*device)] = dispatch_table;
  }

  return result;
}

VK_LAYER_EXPORT void VKAPI_CALL
OrbitCaptureClientDestroyDevice(VkDevice device, const VkAllocationCallbacks* /*allocator*/) {
  absl::WriterMutexLock lock(&mutex);
  device_dispatch.erase(GetKey(device));
}

// --------------------------------------------------------------------------------
// Actual layer implementation
// --------------------------------------------------------------------------------

VK_LAYER_EXPORT VkResult VKAPI_CALL OrbitCaptureClientBeginCommandBuffer(
    VkCommandBuffer command_buffer, const VkCommandBufferBeginInfo* begin_info) {
  absl::ReaderMutexLock lock(&mutex);
  command_buffer_stats[command_buffer] = CommandStats();
  return device_dispatch[GetKey(command_buffer)].BeginCommandBuffer(command_buffer, begin_info);
}

VK_LAYER_EXPORT void VKAPI_CALL OrbitCaptureClientCmdDraw(VkCommandBuffer command_buffer,
                                                          uint32_t vertex_count,
                                                          uint32_t instance_count,
                                                          uint32_t first_vertex,
                                                          uint32_t first_instance) {
  absl::ReaderMutexLock lock(&mutex);

  command_buffer_stats[command_buffer].draw_count++;
  command_buffer_stats[command_buffer].instance_count += instance_count;
  command_buffer_stats[command_buffer].vert_count += instance_count * vertex_count;

  device_dispatch[GetKey(command_buffer)].CmdDraw(command_buffer, vertex_count, instance_count,
                                                  first_vertex, first_instance);
}

VK_LAYER_EXPORT void VKAPI_CALL OrbitCaptureClientCmdDrawIndexed(
    VkCommandBuffer command_buffer, uint32_t index_count, uint32_t instance_count,
    uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {
  absl::ReaderMutexLock lock(&mutex);

  command_buffer_stats[command_buffer].draw_count++;
  command_buffer_stats[command_buffer].instance_count += instance_count;
  command_buffer_stats[command_buffer].vert_count += instance_count * index_count;

  device_dispatch[GetKey(command_buffer)].CmdDrawIndexed(
      command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL
OrbitCaptureClientEndCommandBuffer(VkCommandBuffer command_buffer) {
  absl::ReaderMutexLock lock(&mutex);

  CommandStats& s = command_buffer_stats[command_buffer];
  printf("Command buffer %p ended with %u draws, %u instances and %u vertices", command_buffer,
         s.draw_count, s.instance_count, s.vert_count);

  return device_dispatch[GetKey(command_buffer)].EndCommandBuffer(command_buffer);
}

// --------------------------------------------------------------------------------
// Enumeration function
// --------------------------------------------------------------------------------

VK_LAYER_EXPORT VkResult VKAPI_CALL OrbitCaptureClientEnumerateInstanceLayerProperties(
    uint32_t* property_count, VkLayerProperties* properties) {
  if (property_count != nullptr) {
    *property_count = 1;
  }

  if (properties != nullptr) {
    strcpy(properties->layerName, kLayerName);
    strcpy(properties->description, kLayerDescription);
    properties->implementationVersion = kLayerImplementationVersion;
    properties->specVersion = kLayerSpecVersion;
  }

  return VK_SUCCESS;
}

VK_LAYER_EXPORT VkResult VKAPI_CALL OrbitCaptureClientEnumerateDeviceLayerProperties(
    VkPhysicalDevice /*physical_device*/, uint32_t* property_count, VkLayerProperties* properties) {
  return OrbitCaptureClientEnumerateInstanceLayerProperties(property_count, properties);
}

VK_LAYER_EXPORT VkResult VKAPI_CALL OrbitCaptureClientEnumerateInstanceExtensionProperties(
    const char* layer_name, uint32_t* property_count, VkExtensionProperties* /*properties*/) {
  if (layer_name == nullptr || strcmp(layer_name, kLayerName) != 0) {
    return VK_ERROR_LAYER_NOT_PRESENT;
  }

  // don't expose any extensions
  if (property_count != nullptr) {
    *property_count = 0;
  }
  return VK_SUCCESS;
}

VK_LAYER_EXPORT VkResult VKAPI_CALL OrbitCaptureClientEnumerateDeviceExtensionProperties(
    VkPhysicalDevice physical_device, const char* layer_name, uint32_t* property_count,
    VkExtensionProperties* properties) {
  // pass through any queries that aren't to us
  if (layer_name == nullptr || strcmp(layer_name, kLayerName) != 0) {
    if (physical_device == VK_NULL_HANDLE) {
      return VK_SUCCESS;
    }

    absl::ReaderMutexLock lock(&mutex);
    return instance_dispatch[GetKey(physical_device)].EnumerateDeviceExtensionProperties(
        physical_device, layer_name, property_count, properties);
  }

  // don't expose any extensions
  if (property_count != nullptr) {
    *property_count = 0;
  }
  return VK_SUCCESS;
}

// --------------------------------------------------------------------------------
// GetProcAddr functions, entry points of the layer
// --------------------------------------------------------------------------------

#define GETPROCADDR(func)        \
  if (!strcmp(name, "vk" #func)) \
    return absl::bit_cast<PFN_vkVoidFunction>(&OrbitCaptureClient##func);

VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL
OrbitCaptureClientGetDeviceProcAddr(VkDevice device, const char* name) {
  // device chain functions we intercept
  GETPROCADDR(GetDeviceProcAddr);
  GETPROCADDR(EnumerateDeviceLayerProperties);
  GETPROCADDR(EnumerateDeviceExtensionProperties);
  GETPROCADDR(CreateDevice);
  GETPROCADDR(DestroyDevice);
  GETPROCADDR(BeginCommandBuffer);
  GETPROCADDR(CmdDraw);
  GETPROCADDR(CmdDrawIndexed);
  GETPROCADDR(EndCommandBuffer);

  {
    absl::ReaderMutexLock lock(&mutex);
    return device_dispatch[GetKey(device)].GetDeviceProcAddr(device, name);
  }
}

VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL
OrbitCaptureClientGetInstanceProcAddr(VkInstance instance, const char* name) {
  // instance chain functions we intercept
  GETPROCADDR(GetInstanceProcAddr);
  GETPROCADDR(EnumerateInstanceLayerProperties);
  GETPROCADDR(EnumerateInstanceExtensionProperties);
  GETPROCADDR(CreateInstance);
  GETPROCADDR(DestroyInstance);

  // device chain functions we intercept
  GETPROCADDR(GetDeviceProcAddr);
  GETPROCADDR(EnumerateDeviceLayerProperties);
  GETPROCADDR(EnumerateDeviceExtensionProperties);
  GETPROCADDR(CreateDevice);
  GETPROCADDR(DestroyDevice);
  GETPROCADDR(BeginCommandBuffer);
  GETPROCADDR(CmdDraw);
  GETPROCADDR(CmdDrawIndexed);
  GETPROCADDR(EndCommandBuffer);

  {
    absl::ReaderMutexLock lock(&mutex);
    return instance_dispatch[GetKey(instance)].GetInstanceProcAddr(instance, name);
  }
}
