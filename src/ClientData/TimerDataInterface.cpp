// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/TimerDataInterface.h"

namespace orbit_client_data {

uint64_t GetNextPixelBoundaryTimeNs(uint64_t current_timestamp_ns, uint32_t resolution,
                                    uint64_t start_ns, uint64_t end_ns) {
  uint64_t current_ns_from_start = current_timestamp_ns - start_ns;
  uint64_t total_ns = end_ns - start_ns;

  // Given a resolution of 4000 pixels, we can capture for 53 days without overflowing.
  uint64_t current_pixel = (current_ns_from_start * resolution) / total_ns;
  uint64_t next_pixel = current_pixel + 1;

  // To calculate the timestamp of a pixel boundary, we make a ceiling to be consistent to how we
  // calculate current_pixel.
  uint64_t next_pixel_ns_from_min = (total_ns * next_pixel + resolution - 1) / resolution;

  return start_ns + next_pixel_ns_from_min;
}

}  // namespace orbit_client_data