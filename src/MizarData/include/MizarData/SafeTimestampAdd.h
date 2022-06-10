// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_SAFE_TIMESTAMP_ADD_H_
#define MIZAR_DATA_SAFE_TIMESTAMP_ADD_H_

#include <stdint.h>

#include <cstdint>
#include <limits>

[[nodiscard]] inline uint64_t SafeTimestampsAdd(const uint64_t a, const uint64_t b) {
  const uint64_t sum = a + b;
  if (sum < a || sum < b) return std::numeric_limits<uint64_t>::max();
  return a + b;
}

#endif  // MIZAR_DATA_SAFE_TIMESTAMP_ADD_H_