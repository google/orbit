// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/BatchRenderGroup.h"

namespace orbit_gl {
StencilConfig& StencilConfig::ClipAt(const StencilConfig& parent) {
  if (!parent.enabled) return *this;
  if (!enabled) {
    *this = parent;
    return *this;
  }

  pos[0] = std::min(std::max(pos[0], parent.pos[0]), parent.pos[0] + parent.size[0]);
  pos[1] = std::min(std::max(pos[1], parent.pos[1]), parent.pos[1] + parent.size[1]);

  size[0] = std::max(std::min(size[0], parent.pos[0] + parent.size[0] - pos[0]), 0.f);
  size[1] = std::max(std::min(size[1], parent.pos[1] + parent.size[1] - pos[1]), 0.f);

  return *this;
}
}  // namespace orbit_gl