// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef ORBIT_GL_GL_UTILS_H_
#define ORBIT_GL_GL_UTILS_H_

#include <freetype-gl/mat4.h>
#include <stdint.h>

#include <iostream>

#include "absl/time/time.h"

void CheckGlError();

inline std::ostream& operator<<(std::ostream& os, const ftgl::mat4& mat) {
  os << std::endl;
  os << mat.m00 << "\t" << mat.m01 << "\t" << mat.m02 << "\t" << mat.m03 << std::endl;
  os << mat.m10 << "\t" << mat.m11 << "\t" << mat.m12 << "\t" << mat.m13 << std::endl;
  os << mat.m20 << "\t" << mat.m21 << "\t" << mat.m22 << "\t" << mat.m23 << std::endl;
  os << mat.m30 << "\t" << mat.m31 << "\t" << mat.m32 << "\t" << mat.m33 << std::endl;
  return os;
}

[[nodiscard]] inline absl::Duration TicksToDuration(uint64_t start, uint64_t end) {
  return absl::Nanoseconds(end - start);
}

[[nodiscard]] inline double TicksToMicroseconds(uint64_t start, uint64_t end) {
  return static_cast<double>(end - start) * 0.001;
}

#endif  // ORBIT_GL_GL_UTILS_H_
