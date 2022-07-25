// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <string>
#include <vector>

#include "CoreMath.h"

namespace orbit_gl {

[[nodiscard]] inline Color GetThreadColor(uint64_t id) {
  constexpr unsigned char kAlpha = 255;
  static std::vector<Color> colors{
      Color(231, 68, 53, kAlpha),    // red
      Color(43, 145, 175, kAlpha),   // blue
      Color(185, 117, 181, kAlpha),  // purple
      Color(87, 166, 74, kAlpha),    // green
      Color(215, 171, 105, kAlpha),  // beige
      Color(248, 101, 22, kAlpha)    // orange
  };
  return colors[id % colors.size()];
}

[[nodiscard]] inline Color GetThreadColor(const std::string& str) {
  return GetThreadColor(std::hash<std::string>{}(str));
}

}  // namespace orbit_gl