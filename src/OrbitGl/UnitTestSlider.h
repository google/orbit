// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_UNIT_TEST_SLIDER_H_
#define ORBIT_GL_UNIT_TEST_SLIDER_H_

#include "Batcher.h"
#include "GlSlider.h"
#include "Viewport.h"

namespace orbit_gl {

class UnitTestSlider : public GlSlider {
 public:
  explicit UnitTestSlider(Viewport& viewport, bool is_vertical) : GlSlider(viewport, is_vertical){};
  void Draw(Batcher& /*batcher*/, bool /*is_picked*/) override{};

  bool IsMouseOver() { return is_mouse_over_; }

 protected:
  [[nodiscard]] virtual int GetBarPixelLength() const { return 0; }
};

}  // namespace orbit_gl

#endif