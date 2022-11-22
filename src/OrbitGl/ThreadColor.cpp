// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <vector>

#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

const Color kRedThread{231, 68, 53, 255};
const Color kBlueThread{43, 145, 175, 255};
const Color kPurpleThread{185, 117, 181, 255};
const Color kGreenThread{87, 166, 74, 255};
const Color kBeigeThread{215, 171, 105, 255};
const Color kOrangeThread{248, 101, 22, 255};

[[nodiscard]] Color GetThreadColor(uint32_t id) {
  static std::vector<Color> colors{
      kRedThread, kBlueThread, kPurpleThread, kGreenThread, kBeigeThread, kOrangeThread,
  };
  return colors[id % colors.size()];
}

}  // namespace orbit_gl