// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/Batcher.h"

namespace orbit_gl {
bool operator==(const Batcher::Statistics& lhs, const Batcher::Statistics& rhs) {
  return std::memcmp(&lhs, &rhs, sizeof(Batcher::Statistics)) != 0;
}

}  // namespace orbit_gl