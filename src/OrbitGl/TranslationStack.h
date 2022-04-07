// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRANSLATION_STACK_H_
#define ORBIT_GL_TRANSLATION_STACK_H_

#include <vector>

#include "CoreMath.h"
#include "Geometry.h"

namespace orbit_gl {
class TranslationStack {
 public:
  void PushTranslation(float x, float y, float z = 0.f);
  void PopTranslation();
  [[nodiscard]] bool IsEmpty() const { return translation_stack_.empty(); }

  [[nodiscard]] HasZ<Vec2> TranslateAndFloorVertex(const HasZ<Vec2>& input) const {
    const Vec2 result_shape = input.shape + current_translation_.shape;
    const float result_z = input.z + current_translation_.z;
    return {{floorf(result_shape[0]), floorf(result_shape[1])}, result_z};
  }

 private:
  std::vector<HasZ<Vec2>> translation_stack_;
  HasZ<Vec2> current_translation_{{0.f, 0.f}, 0.f};
};
}  // namespace orbit_gl

#endif