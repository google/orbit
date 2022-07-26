// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "CoreMath.h"

namespace orbit_gl {

[[nodiscard]] inline Color GetThreadColor(uint64_t id);

[[nodiscard]] inline Color GetThreadColor(int64_t id) {
  return GetThreadColor(static_cast<uint64_t>(id));
}

}  // namespace orbit_gl