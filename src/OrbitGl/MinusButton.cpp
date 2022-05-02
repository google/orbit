// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MinusButton.h"

#include "GlCanvas.h"

namespace orbit_gl {

void MinusButton::DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                         const DrawContext& draw_context) {
  Button::DoDraw(primitive_assembler, text_renderer, draw_context);

  primitive_assembler.AddBox(
      MakeBox({GetPos()[0] + kSymbolsPaddingSize, GetPos()[1] + (GetHeight() - kSymbolsWide) / 2.f},
              {GetWidth() - 2 * kSymbolsPaddingSize, kSymbolsWide}),
      GlCanvas::kZValueButton, kSymbolsColor);
}

}  // namespace orbit_gl