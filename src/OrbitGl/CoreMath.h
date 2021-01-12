// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <algorithm>
#include <cmath>

#include "GteVector2.h"
#include "GteVector3.h"
#include "GteVector4.h"

constexpr float kPiFloat = 3.14159265358979f;

using Vec2 = gte::Vector2<float>;
using Vec3 = gte::Vector3<float>;
using Vec4 = gte::Vector4<float>;

using Color = gte::Vector4<unsigned char>;

template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  return std::min(std::max(v, lo), hi);
}
