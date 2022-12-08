// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TrackRenderHelper.h"

#include <GteVector.h>
#include <GteVector2.h>
#include <absl/types/span.h>
#include <stddef.h>

#include <algorithm>
#include <cmath>

#include "OrbitGl/Geometry.h"

namespace {

std::vector<Vec2> RotatePoints(absl::Span<const Vec2> points, float rotation) {
  float cos_r = std::cos(kPiFloat * rotation / 180.f);
  float sin_r = std::sin(kPiFloat * rotation / 180.f);
  std::vector<Vec2> result;
  for (const Vec2& point : points) {
    float x_rotated = cos_r * point[0] + sin_r * point[1];
    float y_rotated = sin_r * point[0] - cos_r * point[1];
    result.emplace_back(x_rotated, y_rotated);
  }
  return result;
}

}  // namespace

namespace orbit_gl {

std::vector<Vec2> GetRoundedCornerMask(float radius, uint32_t num_sides) {
  std::vector<Vec2> points;
  points.emplace_back(0.f, 0.f);
  points.emplace_back(0.f, radius);

  float increment_radians = 0.5f * kPiFloat / static_cast<float>(num_sides);
  for (uint32_t i = 1; i < num_sides; ++i) {
    float angle = kPiFloat + static_cast<float>(i) * increment_radians;
    points.emplace_back(radius * std::cos(angle) + radius, radius * std::sin(angle) + radius);
  }

  points.emplace_back(radius, 0.f);
  return points;
}

void DrawTriangleFan(PrimitiveAssembler& primitive_assembler, absl::Span<const Vec2> points,
                     const Vec2& pos, const Color& color, float rotation, float z,
                     const std::shared_ptr<Pickable>& pickable) {
  if (points.size() < 3) {
    return;
  }

  std::vector<Vec2> rotated_points = RotatePoints(points, rotation);
  Vec2 position(pos[0], pos[1]);
  Vec2 pivot = position + Vec2(rotated_points[0][0], rotated_points[0][1]);

  Vec2 vertices[2];
  vertices[0] = position + Vec2(rotated_points[1][0], rotated_points[1][1]);

  for (size_t i = 1; i < rotated_points.size() - 1; ++i) {
    vertices[i % 2] = position + Vec2(rotated_points[i + 1][0], rotated_points[i + 1][1]);
    Triangle triangle(pivot, vertices[i % 2], vertices[(i + 1) % 2]);
    primitive_assembler.AddTriangle(triangle, z, color, pickable);
  }
}

}  // namespace orbit_gl