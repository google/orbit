// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_DISPATCH_TABLE_H_
#define ORBIT_VULKAN_LAYER_DISPATCH_TABLE_H_

#include "OrbitBase/Logging.h"
#include "absl/base/casts.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vk_layer.h"
#include "vulkan/vk_layer_dispatch_table.h"
#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {

/*
 * A thread-safe dispatch table for Vulkan function look-up.
 *
 * It computes/stores the Vulkan dispatch tables for concrete devices/instances and provides
 * accessors to the functions.
 *
 * For functions provided by extensions it also provide predicate functions to check if the
 * extension is available.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
class DispatchTable {
 public:
  DispatchTable() = default;

  void CreateInstanceDispatchTable(
      VkInstance instance, const PFN_vkGetInstanceProcAddr& next_get_instance_proc_addr_function);
  void RemoveInstanceDispatchTable(VkInstance instance);

  void CreateDeviceDispatchTable(VkDevice device,
                                 const PFN_vkGetDeviceProcAddr& next_get_device_proc_addr_function);
  void RemoveDeviceDispatchTable(VkDevice device);

  template <typename DispatchableType>
  PFN_vkDestroyDevice DestroyDevice(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).DestroyDevice != nullptr);
    return device_dispatch_table_.at(key).DestroyDevice;
  }

  template <typename DispatchableType>
  PFN_vkDestroyInstance DestroyInstance(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(instance_dispatch_table_.contains(key));
    CHECK(instance_dispatch_table_.at(key).DestroyInstance != nullptr);
    return instance_dispatch_table_.at(key).DestroyInstance;
  }

  template <typename DispatchableType>
  PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties(
      const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(instance_dispatch_table_.contains(key));
    CHECK(instance_dispatch_table_.at(key).EnumerateDeviceExtensionProperties != nullptr);
    return instance_dispatch_table_.at(key).EnumerateDeviceExtensionProperties;
  }

  template <typename DispatchableType>
  PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties(
      const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(instance_dispatch_table_.contains(key));
    CHECK(instance_dispatch_table_.at(key).GetPhysicalDeviceProperties != nullptr);
    return instance_dispatch_table_.at(key).GetPhysicalDeviceProperties;
  }

  template <typename DispatchableType>
  PFN_vkGetInstanceProcAddr GetInstanceProcAddr(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(instance_dispatch_table_.contains(key));
    CHECK(instance_dispatch_table_.at(key).GetInstanceProcAddr != nullptr);
    return instance_dispatch_table_.at(key).GetInstanceProcAddr;
  }

  template <typename DispatchableType>
  PFN_vkGetDeviceProcAddr GetDeviceProcAddr(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).GetDeviceProcAddr != nullptr);
    return device_dispatch_table_.at(key).GetDeviceProcAddr;
  }

  template <typename DispatchableType>
  PFN_vkResetCommandPool ResetCommandPool(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).ResetCommandPool != nullptr);
    return device_dispatch_table_.at(key).ResetCommandPool;
  }

  template <typename DispatchableType>
  PFN_vkAllocateCommandBuffers AllocateCommandBuffers(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).AllocateCommandBuffers != nullptr);
    return device_dispatch_table_.at(key).AllocateCommandBuffers;
  }

  template <typename DispatchableType>
  PFN_vkFreeCommandBuffers FreeCommandBuffers(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).FreeCommandBuffers != nullptr);
    return device_dispatch_table_.at(key).FreeCommandBuffers;
  }

  template <typename DispatchableType>
  PFN_vkBeginCommandBuffer BeginCommandBuffer(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).BeginCommandBuffer != nullptr);
    return device_dispatch_table_.at(key).BeginCommandBuffer;
  }

  template <typename DispatchableType>
  PFN_vkEndCommandBuffer EndCommandBuffer(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).EndCommandBuffer != nullptr);
    return device_dispatch_table_.at(key).EndCommandBuffer;
  }

  template <typename DispatchableType>
  PFN_vkResetCommandBuffer ResetCommandBuffer(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).ResetCommandBuffer != nullptr);
    return device_dispatch_table_.at(key).ResetCommandBuffer;
  }

  template <typename DispatchableType>
  PFN_vkGetDeviceQueue GetDeviceQueue(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).GetDeviceQueue != nullptr);
    return device_dispatch_table_.at(key).GetDeviceQueue;
  }

  template <typename DispatchableType>
  PFN_vkGetDeviceQueue2 GetDeviceQueue2(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).GetDeviceQueue2 != nullptr);
    return device_dispatch_table_.at(key).GetDeviceQueue2;
  }

  template <typename DispatchableType>
  PFN_vkQueueSubmit QueueSubmit(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).QueueSubmit != nullptr);
    return device_dispatch_table_.at(key).QueueSubmit;
  }

  template <typename DispatchableType>
  PFN_vkQueuePresentKHR QueuePresentKHR(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).QueuePresentKHR != nullptr);
    return device_dispatch_table_.at(key).QueuePresentKHR;
  }

  template <typename DispatchableType>
  PFN_vkCreateQueryPool CreateQueryPool(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).CreateQueryPool != nullptr);
    return device_dispatch_table_.at(key).CreateQueryPool;
  }

  template <typename DispatchableType>
  PFN_vkResetQueryPoolEXT ResetQueryPoolEXT(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).ResetQueryPoolEXT != nullptr);
    return device_dispatch_table_.at(key).ResetQueryPoolEXT;
  }

  template <typename DispatchableType>
  PFN_vkGetQueryPoolResults GetQueryPoolResults(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).GetQueryPoolResults != nullptr);
    return device_dispatch_table_.at(key).GetQueryPoolResults;
  }

  template <typename DispatchableType>
  PFN_vkCmdWriteTimestamp CmdWriteTimestamp(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).CmdWriteTimestamp != nullptr);
    return device_dispatch_table_.at(key).CmdWriteTimestamp;
  }

  template <typename DispatchableType>
  PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT(
      const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).CmdBeginDebugUtilsLabelEXT != nullptr);
    return device_dispatch_table_.at(key).CmdBeginDebugUtilsLabelEXT;
  }

  template <typename DispatchableType>
  PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT(
      const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).CmdEndDebugUtilsLabelEXT != nullptr);
    return device_dispatch_table_.at(key).CmdEndDebugUtilsLabelEXT;
  }

  template <typename DispatchableType>
  PFN_vkCmdDebugMarkerBeginEXT CmdDebugMarkerBeginEXT(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).CmdDebugMarkerBeginEXT != nullptr);
    return device_dispatch_table_.at(key).CmdDebugMarkerBeginEXT;
  }

  template <typename DispatchableType>
  PFN_vkCmdDebugMarkerEndEXT CmdDebugMarkerEndEXT(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_dispatch_table_.contains(key));
    CHECK(device_dispatch_table_.at(key).CmdDebugMarkerEndEXT != nullptr);
    return device_dispatch_table_.at(key).CmdDebugMarkerEndEXT;
  }

  template <typename DispatchableType>
  bool IsDebugMarkerExtensionSupported(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_supports_debug_marker_extension_.contains(key));
    return device_supports_debug_marker_extension_.at(key);
  }

  template <typename DispatchableType>
  bool IsDebugUtilsExtensionSupported(const DispatchableType& dispatchable_object) {
    absl::ReaderMutexLock lock(&mutex_);
    void* key = GetDispatchTableKey(dispatchable_object);
    CHECK(device_supports_debug_utils_extension_.contains(key));
    return device_supports_debug_utils_extension_.at(key);
  }

 private:
  // In vulkan, every "dispatchable type" has as a very first field in memory a pointer to the
  // internal dispatch table. This pointer is unique per device/instance. So for example
  // for a command buffer allocated on a certain device, this pointer is the same for the buffer
  // and for the device. So we can use that pointer to uniquely map dispatchable types to their
  // dispatch table.
  template <typename DispatchableType>
  void* GetDispatchTableKey(DispatchableType dispatchable_object) {
    return *absl::bit_cast<void**>(dispatchable_object);
  }

  // Dispatch tables required for routing instance and device calls onto the next
  // layer in the dispatch chain among our handling of functions we intercept.
  absl::flat_hash_map<void*, VkLayerInstanceDispatchTable> instance_dispatch_table_;
  absl::flat_hash_map<void*, VkLayerDispatchTable> device_dispatch_table_;
  absl::flat_hash_map<void*, bool> device_supports_debug_marker_extension_;
  absl::flat_hash_map<void*, bool> device_supports_debug_utils_extension_;

  // Must protect access to dispatch tables above by mutex since the Vulkan
  // application may be calling these functions from different threads.
  // However, they are usually filled once (per device/instance) at the beginning
  // and afterwards we only read that data. So we use a read/write lock.
  absl::Mutex mutex_;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_DISPATCH_TABLE_H_
