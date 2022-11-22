// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRANSLATION_STACK_H_
#define ORBIT_GL_TRANSLATION_STACK_H_

#include <GteVector.h>
#include <GteVector2.h>

#include <cmath>
#include <vector>

#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

struct LayeredVec2 {
  Vec2 xy;
  float z{};
};

class TranslationStack {
 public:
  void PushTranslation(float x, float y, float z = 0.f);
  void PopTranslation();
  [[nodiscard]] bool IsEmpty() const { return translation_stack_.empty(); }

  // TODO(b/227341686) if we change the type of z-values to be non-float, the name should be made
  // less verbose, as it would be clear `z` is not floored.
  [[nodiscard]] LayeredVec2 TranslateXYZAndFloorXY(const LayeredVec2& input) const {
    const Vec2 result_shape = input.xy + current_translation_.xy;
    const float result_z = input.z + current_translation_.z;
    return {{std::floor(result_shape[0]), std::floor(result_shape[1])}, result_z};
  }

 private:
  std::vector<LayeredVec2> translation_stack_;
  LayeredVec2 current_translation_{{0.f, 0.f}, 0.f};
};
}  // namespace orbit_gl

#endif