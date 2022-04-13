// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_RENDER_HELPER_H_
#define ORBIT_GL_TRACK_RENDER_HELPER_H_

#include <vector>

#include "CoreMath.h"
#include "PickingManager.h"
#include "PrimitiveAssembler.h"

// Contains free functions used to render track elements such as rounded corners.
namespace orbit_gl {
void DrawTriangleFan(PrimitiveAssembler& primitive_assembler, const std::vector<Vec2>& points,
                     const Vec2& pos, const Color& color, float rotation, float z,
                     std::shared_ptr<Pickable> pickable);
[[nodiscard]] std::vector<Vec2> GetRoundedCornerMask(float radius, uint32_t num_sides);
}  // namespace orbit_gl

#endif