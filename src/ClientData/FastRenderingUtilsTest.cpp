// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>

#include "ClientData/FastRenderingUtils.h"

namespace orbit_client_data {

constexpr uint64_t kStartNs = 100'000'000'000'000ULL;
constexpr uint64_t kEndNs = 200'000'000'000'000ULL;

using GetPixelNumberTest = ::testing::TestWithParam<uint32_t>;
using GetNextPixelBoundaryTimeNsTest = ::testing::TestWithParam<uint32_t>;

TEST_P(GetPixelNumberTest, FirstPixel) {
  const uint32_t resolution = GetParam();
  EXPECT_EQ(GetPixelNumber(kStartNs, resolution, kStartNs, kEndNs), 0);

  const uint64_t last_ns_for_first_pixel = kStartNs + (kEndNs - kStartNs - 1) / resolution;
  EXPECT_EQ(GetPixelNumber(last_ns_for_first_pixel, resolution, kStartNs, kEndNs), 0);
  EXPECT_EQ(GetPixelNumber(last_ns_for_first_pixel + 1, resolution, kStartNs, kEndNs), 1);
}

TEST_P(GetPixelNumberTest, LastPixel) {
  const uint32_t resolution = GetParam();
  EXPECT_EQ(GetPixelNumber(kEndNs - 1, resolution, kStartNs, kEndNs), resolution - 1);
  EXPECT_EQ(GetPixelNumber(kEndNs, resolution, kStartNs, kEndNs), resolution);
}

INSTANTIATE_TEST_SUITE_P(GetPixelNumberTests, GetPixelNumberTest, testing::Values(1, 20, 30, 100));

TEST_P(GetNextPixelBoundaryTimeNsTest, TimestampsAreInRange) {
  const uint32_t resolution = GetParam();
  constexpr uint64_t kVisibleNs = kEndNs - kStartNs;  // 100

  // Calculates `ceil(dividend / divisor)` only using integers assuming dividend and divisor are not
  // 0.
  const auto rounding_up_division = [](uint64_t dividend, uint64_t divisor) -> uint64_t {
    return 1 + (dividend - 1) / divisor;
  };

  constexpr uint32_t kNumberOfTestedTimestamps = 200;
  constexpr uint64_t kStep = (kEndNs - kStartNs - 1) / kNumberOfTestedTimestamps + 1;
  // The max number of nanoseconds per pixel can be calculated using a ceil function.
  const uint64_t max_nanoseconds_per_pixel = rounding_up_division(kVisibleNs, resolution);
  for (uint64_t timestamp_ns = kStartNs; timestamp_ns < kEndNs; timestamp_ns += kStep) {
    const uint64_t next_pixel_ns =
        GetNextPixelBoundaryTimeNs(timestamp_ns, resolution, kStartNs, kEndNs);
    // The timestamp of the next pixel should be between the current one and the current plus the
    // maximum number of nanoseconds per pixel.
    EXPECT_GT(next_pixel_ns, timestamp_ns);
    EXPECT_LE(next_pixel_ns, timestamp_ns + max_nanoseconds_per_pixel);
  }
}

TEST_P(GetNextPixelBoundaryTimeNsTest, NumIterations) {
  const uint32_t resolution = GetParam();

  // Iterating through visible pixels using GetNextPixelBoundaryTimeNs should go once per pixel.
  uint32_t num_iterations = 0;
  uint64_t current_timestamp_ns = kStartNs;
  while (current_timestamp_ns < kEndNs) {
    ++num_iterations;
    current_timestamp_ns =
        GetNextPixelBoundaryTimeNs(current_timestamp_ns, resolution, kStartNs, kEndNs);
  }
  EXPECT_EQ(num_iterations, resolution);
}

TEST(GetNextPixelBoundaryTimeNsTest, ExtremeZoomInBorderCase) {
  constexpr uint64_t kVisibleNs = 100;

  // If there are more visible pixels than visible timestamps, we will have several pixels with the
  // same timestamp. In this case to avoid an infinite loop, the next pixel timestamp should be
  // greater than the one queried, even if the queried pixel is outside the visible time range.
  EXPECT_EQ(GetNextPixelBoundaryTimeNs(kStartNs, kVisibleNs * 10, kStartNs, kStartNs + kVisibleNs),
            kStartNs + 1);
  EXPECT_EQ(GetNextPixelBoundaryTimeNs(kEndNs, kVisibleNs * 10, kStartNs, kStartNs + kVisibleNs),
            kEndNs + 1);
}

INSTANTIATE_TEST_SUITE_P(GetNextPixelBoundaryTimeNsTests, GetNextPixelBoundaryTimeNsTest,
                         testing::Values(1, 20, 30, 100));

}  // namespace orbit_client_data