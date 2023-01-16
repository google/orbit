// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/Batcher.h"

namespace orbit_gl {
bool operator==(const Batcher::Statistics& lhs, const Batcher::Statistics& rhs) {
  return lhs.draw_calls == rhs.draw_calls && lhs.reserved_memory == rhs.reserved_memory &&
         lhs.stored_layers == rhs.stored_layers && lhs.stored_vertices == rhs.stored_vertices;
}

}  // namespace orbit_gl