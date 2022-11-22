// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_QUEUE_MANAGER_H_
#define ORBIT_VULKAN_LAYER_QUEUE_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace orbit_vulkan_layer {

/*
 * This class tracks the mapping from queues to the device (via vkGetDeviceQueue).
 *
 * Note: There is no "untrack" function as there is no such mechanism in Vulkan. We could
 *  use vkDestroyDevice, and iterate over all queues of that device, but as there is usually only
 *  one device anyways and destroy will be called at the end of a game, that would not really help.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
class QueueManager {
 public:
  QueueManager() = default;
  /*
   * Establishes a mapping from the given queue to the given logical device. There must be no
   * existing mapping for that queue to a different device.
   */
  void TrackQueue(VkQueue queue, VkDevice device);

  /*
   * Returns the logical device associated with that queue. It expects that the mapping for that
   * queue is known.
   */
  [[nodiscard]] VkDevice GetDeviceOfQueue(VkQueue queue);

 private:
  absl::Mutex mutex_;
  absl::flat_hash_map<VkQueue, VkDevice> queue_to_device_;
};
}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_QUEUE_MANAGER_H_
