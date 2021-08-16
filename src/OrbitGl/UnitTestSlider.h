// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_UNIT_TEST_SLIDER_H_
#define ORBIT_GL_UNIT_TEST_SLIDER_H_

#include "Batcher.h"
#include "GlSlider.h"
#include "Viewport.h"

namespace orbit_gl {

class UnitTestHorizontalSlider : public GlHorizontalSlider {
 public:
  explicit UnitTestHorizontalSlider(Viewport& viewport) : GlHorizontalSlider(viewport){};
  void Draw(Batcher& /*batcher*/, bool /*is_picked*/) override{};

  bool IsMouseOver() { return is_mouse_over_; }
};

class UnitTestVerticalSlider : public GlVerticalSlider {
 public:
  explicit UnitTestVerticalSlider(Viewport& viewport) : GlVerticalSlider(viewport){};
  void Draw(Batcher& /*batcher*/, bool /*is_picked*/) override{};

  bool IsMouseOver() { return is_mouse_over_; }
};

}  // namespace orbit_gl

#endif