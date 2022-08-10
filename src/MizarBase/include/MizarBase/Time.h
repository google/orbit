// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_TIME_H_
#define MIZAR_BASE_TIME_H_

#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"
#include "stdint.h"

namespace orbit_mizar_base {

// wraps `uint64_t` bears semantics of time in nanoseconds and implements a non-wrapping addition
// TODO(b/242038718) Remove and rely on the explicitly allowed Typedefs arithmetics customisation
struct NonWrappingNanoseconds {
  friend NonWrappingNanoseconds operator+(NonWrappingNanoseconds a, NonWrappingNanoseconds b) {
    const uint64_t sum = a.value + b.value;
    if (sum < a.value || sum < b.value) return {std::numeric_limits<uint64_t>::max()};
    return {sum};
  }

  friend NonWrappingNanoseconds operator-(NonWrappingNanoseconds a, NonWrappingNanoseconds b) {
    ORBIT_CHECK(a.value >= b.value);
    return {a.value - b.value};
  }

  friend NonWrappingNanoseconds operator*(NonWrappingNanoseconds a, uint64_t times) {
    if (std::numeric_limits<uint64_t>::max() / times < a.value) {
      return {std::numeric_limits<uint64_t>::max()};
    }
    return {a.value * times};
  }

  friend bool operator<(NonWrappingNanoseconds a, NonWrappingNanoseconds b) {
    return a.value < b.value;
  }

  friend bool operator==(NonWrappingNanoseconds a, NonWrappingNanoseconds b) {
    return a.value == b.value;
  }

  uint64_t value;
};

struct RelativeTimestampTag : orbit_base::PlusTag<RelativeTimestampTag>,
                              orbit_base::TimesScalarTag<uint64_t> {};

// Represents the time passed since the capture start
using RelativeTimeNs = orbit_base::Typedef<RelativeTimestampTag, NonWrappingNanoseconds>;

constexpr RelativeTimeNs MakeRelativeTimeNs(uint64_t t) { return RelativeTimeNs(std::in_place, t); }

struct TimestampNsTag : orbit_base::MinusTag<RelativeTimestampTag>,
                        orbit_base::PlusTag<RelativeTimestampTag> {};

// Absolute timestamp of the capture start in nanos
using TimestampNs = orbit_base::Typedef<TimestampNsTag, NonWrappingNanoseconds>;

constexpr TimestampNs MakeTimestampNs(uint64_t t) { return TimestampNs(std::in_place, t); }

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_TIME_H_