// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MINUS_BUTTON_H_
#define ORBIT_GL_MINUS_BUTTON_H_

#include "Button.h"

namespace orbit_gl {

// A button including a "minus" symbol.
class MinusButton : public Button {
 public:
  explicit MinusButton(CaptureViewElement* parent, const Viewport* viewport,
                       const TimeGraphLayout* layout)
      : Button(parent, viewport, layout) {}

 protected:
  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MINUS_BUTTON_H_
