// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "BlockChain.h"
#include "CoreMath.h"

class TextBox;

//-----------------------------------------------------------------------------
struct Line {
  Vec3 m_Beg;
  Vec3 m_End;
};

//-----------------------------------------------------------------------------
struct Box {
  Box() = default;
  Box(Vec2 pos, Vec2 size, float z) {
    vertices_[0] = Vec3(pos[0], pos[1], z);
    vertices_[1] = Vec3(pos[0], pos[1] + size[1], z);
    vertices_[2] = Vec3(pos[0] + size[0], pos[1] + size[1], z);
    vertices_[3] = Vec3(pos[0] + size[0], pos[1], z);
  }
  Vec3 vertices_[4];
};

//-----------------------------------------------------------------------------
struct Triangle {
  Triangle() = default;
  Triangle(Vec3 v0, Vec3 v1, Vec3 v2) {
    vertices_[0] = v0;
    vertices_[1] = v1;
    vertices_[2] = v2;
  }
  Vec3 vertices_[3];
};
