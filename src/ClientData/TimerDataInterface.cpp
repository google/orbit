// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/TimerDataInterface.h"

namespace orbit_client_data {

[[nodiscard]] uint64_t GetNextPixelBoundaryTimeNs(uint64_t current_timestamp_ns,
                                                  uint32_t resolution, uint64_t start_ns,
                                                  uint64_t end_ns) {
  uint64_t current_ns_from_start = current_timestamp_ns - start_ns;
  uint64_t total_ns = end_ns - start_ns;

  // Given a resolution of 4000 pixels, we can capture for 53 days without overflowing.
  uint64_t current_pixel = (current_ns_from_start * resolution) / total_ns;
  uint64_t next_pixel = current_pixel + 1;

  // To calculate the timestamp of a pixel boundary, we round to the left similar to how it works in
  // other parts of Orbit.
  uint64_t next_pixel_ns_from_min = total_ns * next_pixel / resolution;

  // Border case when we have a lot of pixels who have the same timestamp (because the number of
  // pixels is less than the nanoseconds in screen). In this case, as we've already drawn in the
  // current_timestamp, the next pixel to draw should have the next timestamp.
  if (next_pixel_ns_from_min == current_ns_from_start) {
    next_pixel_ns_from_min = current_ns_from_start + 1;
  }

  return start_ns + next_pixel_ns_from_min;
}

}  // namespace orbit_client_data