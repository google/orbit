// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_RENDER_HELPER_H_
#define ORBIT_GL_TRACK_RENDER_HELPER_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"

// Contains free functions used to render track elements such as rounded corners.
namespace orbit_gl {
void DrawTriangleFan(PrimitiveAssembler& primitive_assembler, absl::Span<const Vec2> points,
                     const Vec2& pos, const Color& color, float rotation, float z,
                     const std::shared_ptr<Pickable>& pickable);
[[nodiscard]] std::vector<Vec2> GetRoundedCornerMask(float radius, uint32_t num_sides);
}  // namespace orbit_gl

#endif