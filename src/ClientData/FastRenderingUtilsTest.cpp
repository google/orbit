// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/FastRenderingUtils.h"

namespace orbit_client_data {

constexpr uint64_t kStartNs = 100;
constexpr uint64_t kEndNs = 200;
static std::vector<uint32_t> kPixelResolutionsInTest = {1, 20, 30, 50, 100};

TEST(FastRenderingUtils, GetPixelNumber) {
  for (uint32_t resolution : kPixelResolutionsInTest) {
    EXPECT_EQ(GetPixelNumber(kStartNs, resolution, kStartNs, kEndNs), 0);
    EXPECT_EQ(GetPixelNumber(kEndNs - 1, resolution, kStartNs, kEndNs), resolution - 1);
    EXPECT_EQ(GetPixelNumber(kEndNs, resolution, kStartNs, kEndNs), resolution);

    const uint32_t kLastNsForFirstPixel = kStartNs + (kEndNs - kStartNs - 1) / resolution;
    EXPECT_EQ(GetPixelNumber(kLastNsForFirstPixel, resolution, kStartNs, kEndNs), 0);
    EXPECT_EQ(GetPixelNumber(kLastNsForFirstPixel + 1, resolution, kStartNs, kEndNs), 1);
  }
}

TEST(GetNextPixelBoundaryTimeNs, TimestampAreInRange) {
  constexpr uint64_t visible_ns = kEndNs - kStartNs;  // 100

  // Calculates `ceil(dividend / divisor)` only using integers assuming dividend and divisor are not
  // 0.
  const auto rounding_up_division = [](uint64_t dividend, uint64_t divisor) -> uint64_t {
    return 1 + (dividend - 1) / divisor;
  };

  for (uint32_t resolution : kPixelResolutionsInTest) {
    // The max number of nanoseconds per pixel can be calculated using a ceil function.
    const uint64_t max_nanoseconds_per_pixel = rounding_up_division(visible_ns, resolution);
    for (uint64_t timestamp_ns = kStartNs; timestamp_ns < kEndNs; timestamp_ns++) {
      const uint64_t next_pixel_ns =
          GetNextPixelBoundaryTimeNs(timestamp_ns, resolution, kStartNs, kEndNs);
      // The timestamp of the next pixel should be between the current one and the current plus the
      // maximum number of nanoseconds per pixel.
      EXPECT_GT(next_pixel_ns, timestamp_ns);
      EXPECT_LE(next_pixel_ns, timestamp_ns + max_nanoseconds_per_pixel);
    }
  }
}

TEST(GetNextPixelBoundaryTimeNs, NumIterations) {
  // Iterating through visible pixels using GetNextPixelBoundaryTimeNs should go once per pixel.
  for (uint32_t resolution : kPixelResolutionsInTest) {
    uint32_t num_iterations = 0;
    uint64_t current_timestamp_ns = kStartNs;
    while (current_timestamp_ns < kEndNs) {
      ++num_iterations;
      current_timestamp_ns =
          GetNextPixelBoundaryTimeNs(current_timestamp_ns, resolution, kStartNs, kEndNs);
    }
    EXPECT_EQ(num_iterations, resolution);
  }
}

TEST(GetNextPixelBoundaryTimeNs, ExtremeZoomInBorderCase) {
  constexpr uint64_t visible_ns = kEndNs - kStartNs;

  // If there are more visible pixels than visible timestamps, we will have several pixels with the
  // same timestamp. In this case to avoid an infinite loop, the next pixel timestamp should be
  // greater than the one queried.
  EXPECT_EQ(GetNextPixelBoundaryTimeNs(kStartNs, visible_ns * 10, kStartNs, kEndNs), kStartNs + 1);
}

}  // namespace orbit_client_data