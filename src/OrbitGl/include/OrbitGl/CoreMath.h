// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CORE_MATH_H_
#define ORBIT_GL_CORE_MATH_H_

#include <tuple>

#include "GteVector2.h"
#include "GteVector3.h"
#include "GteVector4.h"

constexpr float kPiFloat = 3.14159265358979f;

using Vec2 = gte::Vector2<float>;
using Vec3 = gte::Vector3<float>;
using Vec4 = gte::Vector4<float>;

using Vec2i = gte::Vector2<int>;

using Color = gte::Vector4<unsigned char>;

namespace orbit_gl {

template <typename T>
struct ClosedInterval {
  static_assert(std::is_arithmetic_v<T>,
                "T needs to be an arithmetic type (integral or floating point type).");
  ClosedInterval(T a, T b) : min(a), max(b) {}
  [[nodiscard]] bool Intersects(const ClosedInterval& closed_interval) const {
    return this->min <= closed_interval.max && this->max >= closed_interval.min;
  }

  [[nodiscard]] bool Contains(T value) const { return this->min <= value && this->max >= value; }

  [[nodiscard]] friend bool operator==(const ClosedInterval& lhs, const ClosedInterval& rhs) {
    return std::tie(lhs.min, lhs.max) == std::tie(rhs.min, rhs.max);
  }
  [[nodiscard]] friend bool operator!=(const ClosedInterval& lhs, const ClosedInterval& rhs) {
    return !(lhs == rhs);
  }

  T min;
  T max;
};

[[nodiscard]] inline bool IsInsideRectangle(const Vec2& point, const Vec2& top_left,
                                            const Vec2& size) {
  for (int i = 0; i < 2; i++) {
    if (!ClosedInterval<float>{top_left[i], top_left[i] + size[i]}.Contains(point[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace orbit_gl

#endif  // ORBIT_GL_CORE_MATH_H_
