// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_COMMAND_BUFFER_MANAGER_H_
#define ORBIT_VULKAN_LAYER_COMMAND_BUFFER_MANAGER_H_

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {

/*
 * This class is responsible to track command buffers and command pools.
 * TODO: So far it only tracks the allocation/de-allocation of buffers and pools.
 *  It should probably also track the timestamps inside the buffers (and the markers?)
 *
 * It also tracks which command buffer belongs to which device, which can be used
 * in the `DispatchTable` for function look-up.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks), and can be
 * safely accessed from different threads.
 */
class CommandBufferManager {
 public:
  void TrackCommandPool(VkCommandPool pool);
  void UntrackCommandPool(VkCommandPool pool);
  void TrackCommandBuffers(VkDevice device, VkCommandPool pool,
                           const VkCommandBuffer* command_buffers, uint32_t count);
  void UntrackCommandBuffers(VkDevice device, VkCommandPool pool,
                             const VkCommandBuffer* command_buffers, uint32_t count);

  [[nodiscard]] bool IsCommandPoolTracked(const VkCommandPool& pool);
  [[nodiscard]] bool IsCommandBufferTracked(const VkCommandBuffer& pool);
  [[nodiscard]] const VkDevice& GetDeviceOfCommandBuffer(const VkCommandBuffer& command_buffer);

 private:
  absl::Mutex mutex_;
  absl::flat_hash_set<VkCommandPool> tracked_pools_;
  absl::flat_hash_set<VkCommandBuffer> tracked_command_buffers_;
  absl::flat_hash_map<VkCommandPool, absl::flat_hash_set<VkCommandBuffer>> pool_to_command_buffers_;
  absl::flat_hash_map<VkCommandBuffer, VkDevice> command_buffer_to_device_;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_COMMAND_BUFFER_MANAGER_H_