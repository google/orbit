// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_FAST_RENDERING_UTILS_H_
#define CLIENT_DATA_FAST_RENDERING_UTILS_H_

#include <stdint.h>

namespace orbit_client_data {

// Free Function that will be used for any implementation of
// TimerDataInterface::GetTimersAtDepthDiscretized to get the next pixel timestamp. The query
// assumes a closed-open interval [start_ns, end_ns), so end_ns is not a visible timestamp.
[[nodiscard]] uint64_t GetNextPixelBoundaryTimeNs(uint64_t current_timestamp_ns,
                                                  uint32_t resolution, uint64_t start_ns,
                                                  uint64_t end_ns);

// Free Function that returns the number of the pixel of a particular timestamp. The query
// assumes a closed-open interval [start_ns, end_ns), so end_ns is not a visible timestamp.
[[nodiscard]] uint64_t GetPixelNumber(uint64_t timestamp_ns, uint32_t resolution, uint64_t start_ns,
                                      uint64_t end_ns);

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_FAST_RENDERING_UTILS_H_
