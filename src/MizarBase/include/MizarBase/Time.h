// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_TIME_H_
#define MIZAR_BASE_TIME_H_

#include <utility>

#include "OrbitBase/Typedef.h"
#include "stdint.h"

namespace orbit_mizar_base {

struct RelativeTimestampTag : orbit_base::PlusTag {};

// wraps `uint64_t` bears semantics of time in nanoseconds and implements a non-wrapping addition
struct NonWrappingNanoseconds {
  friend NonWrappingNanoseconds operator+(NonWrappingNanoseconds a, NonWrappingNanoseconds b) {
    const uint64_t sum = a.value + b.value;
    if (sum < a.value || sum < b.value) return {std::numeric_limits<uint64_t>::max()};
    return {sum};
  }

  uint64_t value;
};

// Represents the time passed since the capture start
using RelativeTimeNs = orbit_base::Typedef<RelativeTimestampTag, NonWrappingNanoseconds>;

constexpr RelativeTimeNs MakeRelativeTimeNs(uint64_t t) { return RelativeTimeNs(std::in_place, t); }

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_TIME_H_