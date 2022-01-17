// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_DEVICE_MANAGER_H_
#define ORBIT_VULKAN_LAYER_DEVICE_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <vulkan/vulkan.h>

#include "OrbitBase/Logging.h"

namespace orbit_vulkan_layer {

/*
 * This class maintains a mapping from logical to physical devices (via `vkCreateDevice` and
 * `vkDestroyDevice`).
 * `TrackLogicalDevice` establishes that mapping, while `UntrackLogicalDevice` can be used to
 * release that mapping. To retrieve a logical device's physical device, use `GetPhysicalDevice`.
 *
 * For each physical device, it also provides the `VkPhysicalDeviceProperties`, which can be queried
 * using `GetPhysicalDeviceProperties`. The `DeviceManager` is responsible for retrieving this
 * information (using `vkGetPhysicalDeviceProperties`). These properties can be used e.g. for
 * converting clock cycles to nanosecond timestamps.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
template <class DispatchTable>
class DeviceManager {
 public:
  explicit DeviceManager(DispatchTable* dispatch_table) : dispatch_table_(dispatch_table) {}

  void TrackLogicalDevice(VkPhysicalDevice physical_device, VkDevice logical_device) {
    absl::WriterMutexLock lock(&mutex_);
    ORBIT_CHECK(!logical_device_to_physical_device_.contains(logical_device));
    logical_device_to_physical_device_[logical_device] = physical_device;

    ORBIT_CHECK(!physical_device_to_logical_devices_.contains(physical_device) ||
                !physical_device_to_logical_devices_.at(physical_device).contains(logical_device));
    if (!physical_device_to_logical_devices_.contains(physical_device)) {
      physical_device_to_logical_devices_[physical_device] = {logical_device};
    } else {
      physical_device_to_logical_devices_.at(physical_device).insert(logical_device);
    }

    if (physical_device_to_properties_.contains(physical_device)) {
      return;
    }

    VkPhysicalDeviceProperties properties;
    dispatch_table_->GetPhysicalDeviceProperties(physical_device)(physical_device, &properties);
    physical_device_to_properties_[physical_device] = properties;
  }

  [[nodiscard]] VkPhysicalDevice GetPhysicalDeviceOfLogicalDevice(VkDevice logical_device) {
    absl::ReaderMutexLock lock(&mutex_);
    ORBIT_CHECK(logical_device_to_physical_device_.contains(logical_device));
    return logical_device_to_physical_device_.at(logical_device);
  }

  void UntrackLogicalDevice(VkDevice logical_device) {
    absl::WriterMutexLock lock(&mutex_);
    ORBIT_CHECK(logical_device_to_physical_device_.contains(logical_device));
    VkPhysicalDevice physical_device = logical_device_to_physical_device_.at(logical_device);
    logical_device_to_physical_device_.erase(logical_device);

    ORBIT_CHECK(physical_device_to_logical_devices_.contains(physical_device));
    absl::flat_hash_set<VkDevice>& logical_devices =
        physical_device_to_logical_devices_.at(physical_device);
    ORBIT_CHECK(logical_devices.contains(logical_device));
    logical_devices.erase(logical_device);

    if (!logical_devices.empty()) {
      return;
    }
    physical_device_to_logical_devices_.erase(physical_device);
    physical_device_to_properties_.erase(physical_device);
  }

  [[nodiscard]] VkPhysicalDeviceProperties GetPhysicalDeviceProperties(
      VkPhysicalDevice physical_device) {
    absl::ReaderMutexLock lock(&mutex_);
    ORBIT_CHECK(physical_device_to_properties_.contains(physical_device));
    return physical_device_to_properties_.at(physical_device);
  }

 private:
  absl::Mutex mutex_;
  DispatchTable* dispatch_table_;
  absl::flat_hash_map<VkPhysicalDevice, VkPhysicalDeviceProperties> physical_device_to_properties_;
  absl::flat_hash_map<VkDevice, VkPhysicalDevice> logical_device_to_physical_device_;
  absl::flat_hash_map<VkPhysicalDevice, absl::flat_hash_set<VkDevice>>
      physical_device_to_logical_devices_;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_DEVICE_MANAGER_H_
