// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QueueManager.h"

#include "OrbitBase/Logging.h"

namespace orbit_vulkan_layer {

void QueueManager::TrackQueue(VkQueue queue, VkDevice device) {
  absl::WriterMutexLock lock(&mutex_);
  ORBIT_CHECK(!queue_to_device_.contains(queue) || queue_to_device_.at(queue) == device);
  queue_to_device_.emplace(queue, device);
}

VkDevice QueueManager::GetDeviceOfQueue(VkQueue queue) {
  absl::ReaderMutexLock lock(&mutex_);
  ORBIT_CHECK(queue_to_device_.contains(queue));
  return queue_to_device_.at(queue);
}
}  // namespace orbit_vulkan_layer
