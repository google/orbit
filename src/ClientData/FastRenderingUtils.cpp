// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/FastRenderingUtils.h"

#include <stdint.h>

namespace orbit_client_data {

uint64_t GetPixelNumber(uint64_t timestamp_ns, uint32_t resolution, uint64_t start_ns,
                        uint64_t end_ns) {
  uint64_t current_ns_from_start = timestamp_ns - start_ns;
  uint64_t total_ns = end_ns - start_ns;

  // Given a resolution of 4000 pixels, we can capture for 53 days without overflowing.
  return (current_ns_from_start * resolution) / total_ns;
}

uint64_t GetNextPixelBoundaryTimeNs(uint64_t current_timestamp_ns, uint32_t resolution,
                                    uint64_t start_ns, uint64_t end_ns) {
  uint64_t current_pixel = GetPixelNumber(current_timestamp_ns, resolution, start_ns, end_ns);
  uint64_t next_pixel = current_pixel + 1;

  // Calculates `ceil(dividend / divisor)` only using integers assuming dividend and divisor are not
  // 0.
  const auto rounding_up_division = [](uint64_t dividend, uint64_t divisor) -> uint64_t {
    return 1 + (dividend - 1) / divisor;
  };

  uint64_t total_ns = end_ns - start_ns;
  // To calculate the timestamp of a pixel boundary, we make a cross-multiplication rounding_up to
  // be consistent to how we calculate current_pixel.
  uint64_t next_pixel_ns_from_min = rounding_up_division(total_ns * next_pixel, resolution);

  return start_ns + next_pixel_ns_from_min;
}

}  // namespace orbit_client_data