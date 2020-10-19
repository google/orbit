// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CommandBufferManager.h"

#include "OrbitBase/Logging.h"

namespace orbit_vulkan_layer {

void CommandBufferManager::TrackCommandPool(VkCommandPool pool) {
  absl::WriterMutexLock lock(&mutex_);
  tracked_pools_.insert(pool);
}

void CommandBufferManager::UnTrackCommandPool(VkCommandPool pool) {
  absl::WriterMutexLock lock(&mutex_);
  CHECK(IsCommandPoolTracked(pool));
  tracked_pools_.erase(pool);

  // Remove the command buffers associated with this pool:
  for (const VkCommandBuffer& cb : pool_to_command_buffers_[pool]) {
    CHECK(cb != VK_NULL_HANDLE);
    CHECK(IsCommandBufferTracked(cb));
    tracked_command_buffers_.erase(cb);
  }
  pool_to_command_buffers_.erase(pool);
}

void CommandBufferManager::TrackCommandBuffers(VkDevice device, VkCommandPool pool,
                                               const VkCommandBuffer* command_buffers,
                                               uint32_t count) {
  absl::WriterMutexLock lock(&mutex_);
  CHECK(IsCommandPoolTracked(pool));
  // Create that entry if it does not exist yet.
  absl::flat_hash_set<VkCommandBuffer>& associated_cbs = pool_to_command_buffers_[pool];
  for (uint32_t i = 0; i < count; ++i) {
    const VkCommandBuffer& cb = command_buffers[i];
    CHECK(cb != VK_NULL_HANDLE);
    associated_cbs.insert(cb);
    tracked_command_buffers_.insert(cb);
    command_buffer_to_device_[cb] = device;
  }
}

void CommandBufferManager::UnTrackCommandBuffers(VkDevice device, VkCommandPool pool,
                                                 const VkCommandBuffer* command_buffers,
                                                 uint32_t count) {
  absl::WriterMutexLock lock(&mutex_);
  CHECK(IsCommandPoolTracked(pool));
  absl::flat_hash_set<VkCommandBuffer>& associated_cbs = pool_to_command_buffers_[pool];
  for (uint32_t i = 0; i < count; ++i) {
    const VkCommandBuffer& cb = command_buffers[i];
    CHECK(cb != VK_NULL_HANDLE);
    CHECK(IsCommandBufferTracked(cb));
    associated_cbs.erase(cb);
    tracked_command_buffers_.erase(cb);
    CHECK(command_buffer_to_device_.contains(cb));
    CHECK(command_buffer_to_device_.at(cb) == device);
    command_buffer_to_device_.erase(cb);
  }
}

bool CommandBufferManager::IsCommandPoolTracked(const VkCommandPool& pool) {
  absl::ReaderMutexLock lock(&mutex_);
  return tracked_pools_.find(pool) != tracked_pools_.end();
}

bool CommandBufferManager::IsCommandBufferTracked(const VkCommandBuffer& command_buffer) {
  absl::ReaderMutexLock lock(&mutex_);
  return tracked_command_buffers_.find(command_buffer) != tracked_command_buffers_.end();
}

const VkDevice& CommandBufferManager::GetDeviceOfCommandBuffer(
    const VkCommandBuffer& command_buffer) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(command_buffer_to_device_.contains(command_buffer));
  return command_buffer_to_device_.at(command_buffer);
}

}  // namespace orbit_vulkan_layer