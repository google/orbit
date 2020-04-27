//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
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
