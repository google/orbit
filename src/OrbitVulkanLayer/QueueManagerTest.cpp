// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>

#include <memory>

#include "QueueManager.h"

namespace orbit_vulkan_layer {

TEST(QueueManager, AnNonTrackedQueueCanNotBeRetrieved) {
  QueueManager manager;
  VkQueue queue = {};
  EXPECT_DEATH({ (void)manager.GetDeviceOfQueue(queue); }, "");
}

TEST(QueueManager, AQueueCanBeTrackedAndRetrieved) {
  QueueManager manager;
  VkDevice device = nullptr;
  VkQueue queue = nullptr;

  manager.TrackQueue(queue, device);
  VkDevice result = manager.GetDeviceOfQueue(queue);
  EXPECT_EQ(device, result);
}

}  // namespace orbit_vulkan_layer
