// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GEOMETRY_H_
#define ORBIT_GL_GEOMETRY_H_

#include <algorithm>
#include <utility>

#include "OrbitGl/CoreMath.h"

struct Line {
  Vec2 start_point;
  Vec2 end_point;
};

struct Quad {
  Quad() = default;
  explicit Quad(std::array<Vec2, 4> clockwise_ordered_vertices)
      : vertices(std::move(clockwise_ordered_vertices)) {}

  std::array<Vec2, 4> vertices;
};

[[nodiscard]] inline Quad MakeBox(const Vec2& pos, const Vec2& size) {
  return Quad({Vec2(pos[0], pos[1]), Vec2(pos[0], pos[1] + size[1]),
               Vec2(pos[0] + size[0], pos[1] + size[1]), Vec2(pos[0] + size[0], pos[1])});
}

struct Triangle {
  Triangle() = default;
  Triangle(const Vec2& v0, const Vec2& v1, const Vec2& v2) {
    vertices[0] = v0;
    vertices[1] = v1;
    vertices[2] = v2;
  }
  std::array<Vec2, 3> vertices;
};

#endif