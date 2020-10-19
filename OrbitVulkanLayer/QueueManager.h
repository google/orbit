// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_QUEUE_MANAGER_H_
#define ORBIT_VULKAN_LAYER_QUEUE_MANAGER_H_

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {

/*
 * This class tracks the mapping from queues to the device (via vkGetDeviceQueue).
 *
 * Note: There is no "untrack" function as there is no such mechanism in vulkan. We could
 *  use vkDestroyDevice, and iterate over all queues of that device, but as there is usually only
 *  one device anyways and destroy will be called at the end of a game, that would not really help.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
class QueueManager {
 public:
  QueueManager() = default;
  void TrackQueue(const VkQueue& queue, const VkDevice& device);
  [[nodiscard]] const VkDevice& GetDeviceOfQueue(const VkQueue& queue);

 private:
  absl::Mutex mutex_;
  absl::flat_hash_map<VkQueue, VkDevice> queue_to_device_;
};
}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_QUEUE_MANAGER_H_
