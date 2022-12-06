// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_ABSOLUTE_ADDRESS_H_
#define MIZAR_BASE_ABSOLUTE_ADDRESS_H_

#include <absl/types/span.h>

#include <functional>

#include "OrbitBase/Typedef.h"

namespace orbit_mizar_base {

struct AbsoluteAddressTag {};
// The type represents an absolute address
using AbsoluteAddress = orbit_base::Typedef<AbsoluteAddressTag, const uint64_t>;

template <typename Action>
inline void ForEachFrame(absl::Span<const uint64_t> frames, Action&& action) {
  for (const uint64_t raw_address : frames) {
    std::invoke(action, AbsoluteAddress(raw_address));
  }
}

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_ABSOLUTE_ADDRESS_H_
