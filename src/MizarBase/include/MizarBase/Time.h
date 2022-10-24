// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_TIME_H_
#define MIZAR_BASE_TIME_H_

#include <stdint.h>

#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"

namespace orbit_mizar_base_internal {

inline uint64_t NonWrappingSum(uint64_t a, uint64_t b) {
  const uint64_t sum = a + b;
  if (sum < a || sum < b) return std::numeric_limits<uint64_t>::max();
  return sum;
}

inline uint64_t AbortingOnUnderflowSub(uint64_t a, uint64_t b) {
  ORBIT_CHECK(a >= b);
  return a - b;
}

}  // namespace orbit_mizar_base_internal

namespace orbit_mizar_base {

struct RelativeTimestampTag
    : orbit_base::PlusTag<RelativeTimestampTag, orbit_mizar_base_internal::NonWrappingSum>,
      orbit_base::TimesScalarTag<uint64_t> {};

// Represents the time passed since the capture start
using RelativeTimeNs = orbit_base::Typedef<RelativeTimestampTag, uint64_t>;

struct TimestampNsTag
    : orbit_base::MinusTag<RelativeTimestampTag, orbit_mizar_base_internal::AbortingOnUnderflowSub>,
      orbit_base::PlusTag<RelativeTimestampTag, orbit_mizar_base_internal::NonWrappingSum> {};

// Absolute timestamp of the capture start in nanos
using TimestampNs = orbit_base::Typedef<TimestampNsTag, uint64_t>;

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_TIME_H_