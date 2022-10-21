// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_ALIGN_H_
#define ORBIT_BASE_ALIGN_H_

#include <cstdint>

namespace orbit_base {

// alignment must be a power of 2
template <uint64_t alignment>
constexpr uint64_t AlignUp(uint64_t value) {
  static_assert((alignment & (alignment - 1)) == 0);
  return (value + (alignment - 1)) & ~(alignment - 1);
}

// alignment must be a power of 2
template <uint64_t alignment>
constexpr uint64_t AlignDown(uint64_t value) {
  static_assert((alignment & (alignment - 1)) == 0);
  return value & ~(alignment - 1);
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_ALIGN_H_
