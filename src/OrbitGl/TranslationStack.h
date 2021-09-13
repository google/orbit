// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRANSFORMATION_STACK_H_
#define ORBIT_GL_TRANSFORMATION_STACK_H_

#include <vector>

#include "CoreMath.h"

namespace orbit_gl {
class TranslationStack {
 public:
  void PushTranslation(float x, float y, float z = 0.f);
  void PopTranslation();
  [[nodiscard]] bool IsEmpty() { return translation_stack_.empty(); }

  [[nodiscard]] Vec3 TransformAndFloorVertex(const Vec3& input) const;

 private:
  std::vector<Vec3> translation_stack_;
  Vec3 current_translation_ = Vec3(0.f, 0.f, 0.f);
};
}  // namespace orbit_gl

#endif