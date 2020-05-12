//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <algorithm>
#include <math.h>

#include "GteVector2.h"
#include "GteVector3.h"
#include "GteVector4.h"

constexpr float kPiFloat = 3.14159265358979f;

typedef gte::Vector2<float> Vec2;
typedef gte::Vector3<float> Vec3;
typedef gte::Vector4<float> Vec4;
typedef gte::Vector2<int> Vec2i;

typedef gte::Vector4<unsigned char> Color;

template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  return std::min(std::max(v, lo), hi);
}
