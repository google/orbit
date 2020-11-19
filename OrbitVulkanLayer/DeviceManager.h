// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_PHYSICAL_DEVICE_MANAGER_H_
#define ORBIT_VULKAN_LAYER_PHYSICAL_DEVICE_MANAGER_H_

#include "OrbitBase/Logging.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {

/*
 * This class maintains a mapping from logical to physical devices (via `vkCreateDevice` and
 * `vkDestroyDevice`).
 * `TrackLogicalDevice` establishes that mapping, while `UntrackLogicalDevice` can be used to
 * release that mapping. To retrieve a logical device's physical device, use `GetPhysicalDevice`.
 *
 * For each physical device, it also provides the `VkPhysicalDeviceProperties`, which can be queried
 * using `GetPhysicalDeviceProperties`. The `DeviceManager` is responsible for retrieving this
 * information (using `vkGetPhysicalDeviceProperties`). These properties can be e.g. used for
 * converting clock cycles to nanosecond timestamps.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
template <class DispatchTable>
class DeviceManager {
 public:
  explicit DeviceManager(DispatchTable* dispatch_table) : dispatch_table_(dispatch_table) {}

  void TrackLogicalDevice(VkPhysicalDevice physical_device, VkDevice device) {
    absl::WriterMutexLock lock(&mutex_);
    CHECK(!device_to_physical_device_.contains(device));
    device_to_physical_device_[device] = physical_device;

    CHECK(!physical_device_to_logical_devices_.contains(physical_device) ||
          !physical_device_to_logical_devices_.at(physical_device).contains(device));
    if (!physical_device_to_logical_devices_.contains(physical_device)) {
      physical_device_to_logical_devices_[physical_device] = {device};
    } else {
      physical_device_to_logical_devices_.at(physical_device).insert(device);
    }

    if (physical_device_to_properties_.contains(physical_device)) {
      return;
    }

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
    VkPhysicalDevice physical_device = device_to_physical_device_.at(device);
    device_to_physical_device_.erase(device);

    CHECK(physical_device_to_logical_devices_.contains(physical_device));
    auto& logical_devices = physical_device_to_logical_devices_.at(physical_device);
    CHECK(logical_devices.contains(device));
    logical_devices.erase(device);

    if (!logical_devices.empty()) {
      return;
    }
    physical_device_to_logical_devices_.erase(physical_device);
    physical_device_to_properties_.erase(device_to_physical_device_.at(device));
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
  absl::flat_hash_map<VkPhysicalDevice, absl::flat_hash_set<VkDevice>>
      physical_device_to_logical_devices_;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_PHYSICAL_DEVICE_MANAGER_H_
