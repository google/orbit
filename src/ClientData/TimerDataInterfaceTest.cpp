// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/TimerDataInterface.h"

namespace orbit_client_data {

TEST(TimerData, GetNextPixelBoundaryTimeNs) {
  const uint64_t kStartNs = 100;
  const uint64_t kEndNs = 200;
  uint64_t num_visible_pixels = kEndNs - kStartNs;  // 100

  std::vector<uint32_t> kPixelResolutionsInTest = {1, 20, 30, 50, 100};

  // First we test for different resolutions that the next pixel is greater than the current one but
  // also not greater than the maximum number of nanoseconds per pixel.
  for (uint32_t resolution : kPixelResolutionsInTest) {
    // The max number of nanoseconds per pixel can be calculated using a ceil function.
    uint32_t max_nanoseconds_per_pixel = (num_visible_pixels + resolution - 1) / resolution;
    for (uint64_t timestamp_ns = kStartNs; timestamp_ns < kEndNs; timestamp_ns++) {
      uint64_t next_pixel_ns =
          GetNextPixelBoundaryTimeNs(timestamp_ns, resolution, kStartNs, kEndNs);
      EXPECT_GT(next_pixel_ns, timestamp_ns);
      EXPECT_LE(next_pixel_ns, timestamp_ns + max_nanoseconds_per_pixel);
    }
  }

  // Second we test that iterating through visible pixels using GetNextPixelBoundaryTimeNs goes once
  // per pixel.
  for (uint32_t resolution : kPixelResolutionsInTest) {
    int it = 0;
    uint64_t current_timestamp_ns = kStartNs;
    while (current_timestamp_ns < kEndNs) {
      ++it;
      current_timestamp_ns =
          GetNextPixelBoundaryTimeNs(current_timestamp_ns, resolution, kStartNs, kEndNs);
    }
    EXPECT_EQ(it, resolution);
  }

  // If there are more visible pixels than visible timestamps, we will have several pixels with the
  // same timestamp. In this case to avoid an infinite loop, the next pixel timestamp should be
  // greater than the one queried.
  EXPECT_EQ(GetNextPixelBoundaryTimeNs(kStartNs, num_visible_pixels * 10, kStartNs, kEndNs),
            kStartNs + 1);
}

}  // namespace orbit_client_data