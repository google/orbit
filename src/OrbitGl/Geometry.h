// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GEOMETRY_H_
#define ORBIT_GL_GEOMETRY_H_

#include <algorithm>
#include <utility>

#include "CoreMath.h"

struct Line {
  Vec3 start_point;
  Vec3 end_point;
};

// TODO(b/227748244) Tetragon should store four Vec2
struct Tetragon {
  Tetragon() = default;
  explicit Tetragon(std::array<Vec3, 4> clockwise_ordered_vertices)
      : vertices(std::move(clockwise_ordered_vertices)) {}
  explicit Tetragon(std::array<Vec2, 4> clockwise_ordered_vertices, float z = 0) {
    std::transform(clockwise_ordered_vertices.begin(), clockwise_ordered_vertices.end(),
                   vertices.begin(), [z](Vec2& v) { return Vec2ToVec3(v, z); });
  }

  std::array<Vec3, 4> vertices;
};

[[nodiscard]] inline Tetragon MakeBox(const Vec2& pos, const Vec2& size, float z) {
  return Tetragon{{Vec3(pos[0], pos[1], z), Vec3(pos[0], pos[1] + size[1], z),
                   Vec3(pos[0] + size[0], pos[1] + size[1], z), Vec3(pos[0] + size[0], pos[1], z)}};
}

struct Triangle {
  Triangle() = default;
  Triangle(Vec3 v0, Vec3 v1, Vec3 v2) {
    vertices[0] = v0;
    vertices[1] = v1;
    vertices[2] = v2;
  }
  Vec3 vertices[3];
};

#endif