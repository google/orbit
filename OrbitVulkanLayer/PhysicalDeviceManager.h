// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_PHYSICAL_DEVICE_MANAGER_H_
#define ORBIT_VULKAN_LAYER_PHYSICAL_DEVICE_MANAGER_H_

#include "OrbitBase/Logging.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {

/*
 * This class maintains a mapping from logical to physical devices (via `vkCreateDevice` and
 * `vkDestroyDevice`).
 * `TrackPhysicalDevice` establishes that mapping, while `UntrackLogicalDevice` can used to release
 * that mapping. To retrieve a logical device's physical device, use `GetPhysicalDevice`.
 *
 * For each physical device, it also provides the `VkPhysicalDevice`, which can be queried using
 * `GetPhysicalDeviceProperties`. This class is responsible to retrieve this information
 * (using `vkGetPhysicalDeviceProperties`). These properties can be e.g. used for converting clock
 * cycles in nanosecond timestamps.
 *
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
template <class DispatchTable>
class PhysicalDeviceManager {
 public:
  explicit PhysicalDeviceManager(DispatchTable* dispatch_table) : dispatch_table_(dispatch_table) {}

  void TrackPhysicalDevice(VkPhysicalDevice physical_device, VkDevice device) {
    absl::WriterMutexLock lock(&mutex_);
    device_to_physical_device_[device] = physical_device;
    VkPhysicalDeviceProperties properties;
    dispatch_table_->GetPhysicalDeviceProperties(physical_device)(physical_device, &properties);
    physical_device_to_properties_[physical_device] = properties;
  }

  [[nodiscard]] VkPhysicalDevice GetPhysicalDeviceOfLogicalDevice(VkDevice device) {
    absl::ReaderMutexLock lock(&mutex_);
    CHECK(device_to_physical_device_.contains(device));
    return device_to_physical_device_.at(device);
  }

  void UntrackLogicalDevice(VkDevice device) {
    absl::WriterMutexLock lock(&mutex_);
    CHECK(device_to_physical_device_.contains(device));
    physical_device_to_properties_.erase(device_to_physical_device_.at(device));
    device_to_physical_device_.erase(device);
  }

  [[nodiscard]] VkPhysicalDeviceProperties GetPhysicalDeviceProperties(VkPhysicalDevice device) {
    LOG("GetPhysicalDeviceProperties");
    absl::ReaderMutexLock lock(&mutex_);
    CHECK(physical_device_to_properties_.contains(device));
    return physical_device_to_properties_.at(device);
  }

 private:
  absl::Mutex mutex_;
  DispatchTable* dispatch_table_;
  absl::flat_hash_map<VkPhysicalDevice, VkPhysicalDeviceProperties> physical_device_to_properties_;
  absl::flat_hash_map<VkDevice, VkPhysicalDevice> device_to_physical_device_;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_PHYSICAL_DEVICE_MANAGER_H_
