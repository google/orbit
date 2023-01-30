// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/BatchRenderGroup.h"

#include <GteVector.h>

#include <algorithm>

namespace orbit_gl {
StencilConfig ClipStencil(const StencilConfig& child, const StencilConfig& parent) {
  if (!parent.enabled) return child;
  if (!child.enabled) {
    return parent;
  }

  StencilConfig result = child;

  Vec2 bottom_right = std::max(Vec2(child.pos + child.size), child.pos);
  Vec2 parent_bottom_right = std::max(Vec2(parent.pos + parent.size), parent.pos);

  Vec2 pos = std::clamp(child.pos, parent.pos, parent_bottom_right);
  bottom_right = std::clamp(bottom_right, parent.pos, parent_bottom_right);

  Vec2 size = bottom_right - pos;

  result.pos = pos;
  result.size = size;

  return result;
}
}  // namespace orbit_gl