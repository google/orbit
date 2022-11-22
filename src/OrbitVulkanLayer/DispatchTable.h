// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_DISPATCH_TABLE_H_
#define ORBIT_VULKAN_LAYER_DISPATCH_TABLE_H_

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
// clang-format off
#include <vulkan/vulkan.h> // IWYU pragma: keep
#include <vulkan/vk_layer_dispatch_table.h> // IWYU pragma: keep
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "OrbitBase/Logging.h"
// clang-format on

namespace orbit_vulkan_layer {

/*
 * A thread-safe dispatch table for Vulkan function look-up.
 *
 * It computes/stores the Vulkan dispatch tables for concrete devices/instances and provides
 * accessors to the functions.
 *
 * For functions provided by extensions it also provides predicate functions to check if the
 * extension is available.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
class DispatchTable {
 public:
  DispatchTable() = default;

  void CreateInstanceDispatchTable(VkInstance instance,
                                   PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function);
  void RemoveInstanceDispatchTable(VkInstance instance);

  void CreateDeviceDispatchTable(VkDevice device,
                                 PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function);
  void RemoveDeviceDispatchTable(VkDevice device);

  template <typename DispatchableType>
  PFN_vkDestroyDevice DestroyDevice(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).DestroyDevice != nullptr);
      return device_dispatch_table_.at(key).DestroyDevice;
    }
  }

  template <typename DispatchableType>
  PFN_vkDestroyInstance DestroyInstance(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).DestroyInstance != nullptr);
      return instance_dispatch_table_.at(key).DestroyInstance;
    }
  }

  template <typename DispatchableType>
  PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).EnumerateDeviceExtensionProperties != nullptr);
      return instance_dispatch_table_.at(key).EnumerateDeviceExtensionProperties;
    }
  }

  template <typename DispatchableType>
  PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).GetPhysicalDeviceProperties != nullptr);
      return instance_dispatch_table_.at(key).GetPhysicalDeviceProperties;
    }
  }

  template <typename DispatchableType>
  PFN_vkGetInstanceProcAddr GetInstanceProcAddr(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).GetInstanceProcAddr != nullptr);
      return instance_dispatch_table_.at(key).GetInstanceProcAddr;
    }
  }

  template <typename DispatchableType>
  PFN_vkGetDeviceProcAddr GetDeviceProcAddr(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).GetDeviceProcAddr != nullptr);
      return device_dispatch_table_.at(key).GetDeviceProcAddr;
    }
  }

  template <typename DispatchableType>
  PFN_vkResetCommandPool ResetCommandPool(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).ResetCommandPool != nullptr);
      return device_dispatch_table_.at(key).ResetCommandPool;
    }
  }

  template <typename DispatchableType>
  PFN_vkAllocateCommandBuffers AllocateCommandBuffers(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).AllocateCommandBuffers != nullptr);
      return device_dispatch_table_.at(key).AllocateCommandBuffers;
    }
  }

  template <typename DispatchableType>
  PFN_vkFreeCommandBuffers FreeCommandBuffers(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).FreeCommandBuffers != nullptr);
      return device_dispatch_table_.at(key).FreeCommandBuffers;
    }
  }

  template <typename DispatchableType>
  PFN_vkBeginCommandBuffer BeginCommandBuffer(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).BeginCommandBuffer != nullptr);
      return device_dispatch_table_.at(key).BeginCommandBuffer;
    }
  }

  template <typename DispatchableType>
  PFN_vkEndCommandBuffer EndCommandBuffer(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).EndCommandBuffer != nullptr);
      return device_dispatch_table_.at(key).EndCommandBuffer;
    }
  }

  template <typename DispatchableType>
  PFN_vkResetCommandBuffer ResetCommandBuffer(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).ResetCommandBuffer != nullptr);
      return device_dispatch_table_.at(key).ResetCommandBuffer;
    }
  }

  template <typename DispatchableType>
  PFN_vkGetDeviceQueue GetDeviceQueue(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).GetDeviceQueue != nullptr);
      return device_dispatch_table_.at(key).GetDeviceQueue;
    }
  }

  template <typename DispatchableType>
  PFN_vkGetDeviceQueue2 GetDeviceQueue2(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).GetDeviceQueue2 != nullptr);
      return device_dispatch_table_.at(key).GetDeviceQueue2;
    }
  }

  template <typename DispatchableType>
  PFN_vkQueueSubmit QueueSubmit(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).QueueSubmit != nullptr);
      return device_dispatch_table_.at(key).QueueSubmit;
    }
  }

  template <typename DispatchableType>
  PFN_vkQueuePresentKHR QueuePresentKHR(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).QueuePresentKHR != nullptr);
      return device_dispatch_table_.at(key).QueuePresentKHR;
    }
  }

  template <typename DispatchableType>
  PFN_vkCreateQueryPool CreateQueryPool(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CreateQueryPool != nullptr);
      return device_dispatch_table_.at(key).CreateQueryPool;
    }
  }

  template <typename DispatchableType>
  PFN_vkDestroyQueryPool DestroyQueryPool(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).DestroyQueryPool != nullptr);
      return device_dispatch_table_.at(key).DestroyQueryPool;
    }
  }

  template <typename DispatchableType>
  PFN_vkResetQueryPoolEXT ResetQueryPoolEXT(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).ResetQueryPoolEXT != nullptr);
      return device_dispatch_table_.at(key).ResetQueryPoolEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkGetQueryPoolResults GetQueryPoolResults(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).GetQueryPoolResults != nullptr);
      return device_dispatch_table_.at(key).GetQueryPoolResults;
    }
  }

  template <typename DispatchableType>
  PFN_vkCmdWriteTimestamp CmdWriteTimestamp(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CmdWriteTimestamp != nullptr);
      return device_dispatch_table_.at(key).CmdWriteTimestamp;
    }
  }

  // ----------------------------------------------------------------------------
  // Debug marker extension:
  // ----------------------------------------------------------------------------
  template <typename DispatchableType>
  PFN_vkCmdDebugMarkerBeginEXT CmdDebugMarkerBeginEXT(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CmdDebugMarkerBeginEXT != nullptr);
      return device_dispatch_table_.at(key).CmdDebugMarkerBeginEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkCmdDebugMarkerEndEXT CmdDebugMarkerEndEXT(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CmdDebugMarkerEndEXT != nullptr);
      return device_dispatch_table_.at(key).CmdDebugMarkerEndEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkCmdDebugMarkerInsertEXT CmdDebugMarkerInsertEXT(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CmdDebugMarkerInsertEXT != nullptr);
      return device_dispatch_table_.at(key).CmdDebugMarkerInsertEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkDebugMarkerSetObjectTagEXT DebugMarkerSetObjectTagEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).DebugMarkerSetObjectTagEXT != nullptr);
      return device_dispatch_table_.at(key).DebugMarkerSetObjectTagEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).DebugMarkerSetObjectNameEXT != nullptr);
      return device_dispatch_table_.at(key).DebugMarkerSetObjectNameEXT;
    }
  }

  template <typename DispatchableType>
  bool IsDebugMarkerExtensionSupported(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_supports_debug_marker_extension_.contains(key));
      return device_supports_debug_marker_extension_.at(key);
    }
  }

  // ----------------------------------------------------------------------------
  // Debug utils extension:
  // ----------------------------------------------------------------------------
  template <typename DispatchableType>
  PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CmdBeginDebugUtilsLabelEXT != nullptr);
      return device_dispatch_table_.at(key).CmdBeginDebugUtilsLabelEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CmdEndDebugUtilsLabelEXT != nullptr);
      return device_dispatch_table_.at(key).CmdEndDebugUtilsLabelEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).CmdInsertDebugUtilsLabelEXT != nullptr);
      return device_dispatch_table_.at(key).CmdInsertDebugUtilsLabelEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).SetDebugUtilsObjectNameEXT != nullptr);
      return device_dispatch_table_.at(key).SetDebugUtilsObjectNameEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).SetDebugUtilsObjectTagEXT != nullptr);
      return device_dispatch_table_.at(key).SetDebugUtilsObjectTagEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkQueueBeginDebugUtilsLabelEXT QueueBeginDebugUtilsLabelEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).QueueBeginDebugUtilsLabelEXT != nullptr);
      return device_dispatch_table_.at(key).QueueBeginDebugUtilsLabelEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkQueueEndDebugUtilsLabelEXT QueueEndDebugUtilsLabelEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).QueueEndDebugUtilsLabelEXT != nullptr);
      return device_dispatch_table_.at(key).QueueEndDebugUtilsLabelEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkQueueInsertDebugUtilsLabelEXT QueueInsertDebugUtilsLabelEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_dispatch_table_.contains(key));
      ORBIT_CHECK(device_dispatch_table_.at(key).QueueInsertDebugUtilsLabelEXT != nullptr);
      return device_dispatch_table_.at(key).QueueInsertDebugUtilsLabelEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).CreateDebugUtilsMessengerEXT != nullptr);
      return instance_dispatch_table_.at(key).CreateDebugUtilsMessengerEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).DestroyDebugUtilsMessengerEXT != nullptr);
      return instance_dispatch_table_.at(key).DestroyDebugUtilsMessengerEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).SubmitDebugUtilsMessageEXT != nullptr);
      return instance_dispatch_table_.at(key).SubmitDebugUtilsMessageEXT;
    }
  }

  template <typename DispatchableType>
  bool IsDebugUtilsExtensionSupported(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(device_supports_debug_utils_extension_.contains(key));
      return device_supports_debug_utils_extension_.at(key);
    }
  }

  bool IsDebugUtilsExtensionSupported(VkInstance instance) {
    void* key = GetDispatchTableKey(instance);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_supports_debug_utils_extension_.contains(key));
      return instance_supports_debug_utils_extension_.at(key);
    }
  }

  // ----------------------------------------------------------------------------
  // Debug report extension:
  // ----------------------------------------------------------------------------
  template <typename DispatchableType>
  PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).CreateDebugReportCallbackEXT != nullptr);
      return instance_dispatch_table_.at(key).CreateDebugReportCallbackEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT(
      DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).DestroyDebugReportCallbackEXT != nullptr);
      return instance_dispatch_table_.at(key).DestroyDebugReportCallbackEXT;
    }
  }

  template <typename DispatchableType>
  PFN_vkDebugReportMessageEXT DebugReportMessageEXT(DispatchableType dispatchable_object) {
    void* key = GetDispatchTableKey(dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatch_table_.contains(key));
      ORBIT_CHECK(instance_dispatch_table_.at(key).DebugReportMessageEXT != nullptr);
      return instance_dispatch_table_.at(key).DebugReportMessageEXT;
    }
  }

  bool IsDebugReportExtensionSupported(VkInstance instance) {
    void* key = GetDispatchTableKey(instance);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_supports_debug_report_extension_.contains(key));
      return instance_supports_debug_report_extension_.at(key);
    }
  }

  template <typename DispatchableType>
  VkInstance GetInstance(DispatchableType instance_dispatchable_object) {
    void* key = GetDispatchTableKey(instance_dispatchable_object);
    {
      absl::ReaderMutexLock lock(&mutex_);
      ORBIT_CHECK(instance_dispatchable_object_to_instance_.contains(key));
      return instance_dispatchable_object_to_instance_.at(key);
    }
  }

 private:
  // Vulkan has the concept of "dispatchable types". Basically every Vulkan type whose objects can
  // be associated with a VkInstance or VkDevice are "dispatchable". As an example, both a device
  // and a command buffer corresponding to this device are "dispatchable".
  // Every "dispatchable type" has as a very first field in memory a pointer to the internal
  // dispatch table. This pointer is unique per device/instance. So for example for a command buffer
  // allocated on a certain device, this pointer is the same for the buffer and for the device. So
  // we can use that pointer to uniquely map "dispatchable types" to their dispatch table.
  template <typename DispatchableType>
  void* GetDispatchTableKey(DispatchableType dispatchable_object) {
    void* dispatch;
    memcpy(&dispatch, dispatchable_object, sizeof(void*));
    return dispatch;
  }

  // Dispatch tables required for routing instance and device calls onto the next
  // layer in the dispatch chain among our handling of functions we intercept.
  absl::flat_hash_map<void*, VkLayerInstanceDispatchTable> instance_dispatch_table_;
  absl::flat_hash_map<void*, VkLayerDispatchTable> device_dispatch_table_;

  absl::flat_hash_map<void*, bool> device_supports_debug_marker_extension_;
  absl::flat_hash_map<void*, bool> device_supports_debug_utils_extension_;
  absl::flat_hash_map<void*, bool> instance_supports_debug_utils_extension_;
  absl::flat_hash_map<void*, bool> instance_supports_debug_report_extension_;

  absl::flat_hash_map<void*, VkInstance> instance_dispatchable_object_to_instance_;

  // Must protect access to dispatch tables above by mutex since the Vulkan
  // application may be calling these functions from different threads.
  // However, they are usually filled once (per device/instance) at the beginning
  // and afterwards we only read that data. So we use read/write locks.
  absl::Mutex mutex_;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_DISPATCH_TABLE_H_
