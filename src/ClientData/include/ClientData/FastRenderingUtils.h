// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_FAST_RENDERING_UTILS_H_
#define CLIENT_DATA_FAST_RENDERING_UTILS_H_

#include <stdint.h>

namespace orbit_client_data {

// Free Function that returns the number of the pixel of a particular timestamp. The query
// assumes a closed-open interval [start_ns, end_ns), so end_ns is not a visible timestamp.
[[nodiscard]] inline uint64_t GetPixelNumber(uint64_t timestamp_ns, uint32_t resolution,
                                             uint64_t start_ns, uint64_t end_ns) {
  uint64_t current_ns_from_start = timestamp_ns - start_ns;
  uint64_t total_ns = end_ns - start_ns;

  // Given a resolution of 4000 pixels, we can capture for 53 days without overflowing.
  return (current_ns_from_start * resolution) / total_ns;
}

// Free Function that will be used for any implementation of
// TimerDataInterface::GetTimersAtDepthDiscretized to get the next pixel timestamp. The query
// assumes a closed-open interval [start_ns, end_ns), so end_ns is not a visible timestamp.
[[nodiscard]] inline uint64_t GetNextPixelBoundaryTimeNs(uint64_t current_timestamp_ns,
                                                         uint32_t resolution, uint64_t start_ns,
                                                         uint64_t end_ns) {
  uint64_t current_pixel = GetPixelNumber(current_timestamp_ns, resolution, start_ns, end_ns);
  uint64_t next_pixel = current_pixel + 1;

  uint64_t total_ns = end_ns - start_ns;
  // To calculate the timestamp of a pixel boundary, we make a cross-multiplication rounding_up to
  // be consistent to how we calculate current_pixel. We use that `ceil(a/b) = 1 + (a - 1) / b`,
  // assuming a, b are integers greater than 0.
  uint64_t next_pixel_ns_from_min = 1 + (total_ns * next_pixel - 1) / resolution;

  return start_ns + next_pixel_ns_from_min;
}

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_FAST_RENDERING_UTILS_H_
