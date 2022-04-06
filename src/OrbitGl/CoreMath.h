// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CORE_MATH_H_
#define ORBIT_GL_CORE_MATH_H_

#include "GteVector2.h"
#include "GteVector3.h"
#include "GteVector4.h"

constexpr float kPiFloat = 3.14159265358979f;

using Vec2 = gte::Vector2<float>;
using Vec3 = gte::Vector3<float>;
using Vec4 = gte::Vector4<float>;

using Vec2i = gte::Vector2<int>;
using Vec3i = gte::Vector3<int>;
using Vec4i = gte::Vector4<int>;

using Color = gte::Vector4<unsigned char>;

[[nodiscard]] inline Vec2 Vec3ToVec2(const Vec3& v) { return {v[0], v[1]}; }
[[nodiscard]] inline Vec3 Vec2ToVec3(Vec2 vertex, float z = 0) { return {vertex[0], vertex[1], z}; }

#endif  // ORBIT_GL_CORE_MATH_H_
