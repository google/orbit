// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_QT_TEXT_RENDERER_H_
#define ORBIT_GL_QT_TEXT_RENDERER_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "CoreMath.h"
#include "PrimitiveAssembler.h"
#include "TextRenderer.h"

namespace orbit_gl {

class QtTextRenderer : public TextRenderer {
 public:
  void Init() override{};
  void Clear() override { stored_text_.clear(); };

  void RenderLayer(QPainter* painter, float layer) override;
  void RenderDebug(PrimitiveAssembler* primitive_assembler) override;
  [[nodiscard]] std::vector<float> GetLayers() const override;

  void AddText(const char* text, float x, float y, float z, TextFormatting formatting) override;
  void AddText(const char* text, float x, float y, float z, TextFormatting formatting,
               Vec2* out_text_pos, Vec2* out_text_size) override;

  float AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                        TextFormatting formatting,
                                        size_t trailing_chars_length) override;

  [[nodiscard]] float GetStringWidth(const char* text, uint32_t font_size) override;
  [[nodiscard]] float GetStringHeight(const char* text, uint32_t font_size) override;

 private:
  struct StoredText {
    StoredText() = default;
    StoredText(const char* text, float x, float y, float z, TextFormatting formatting) {
      this->text = text;
      this->x = x;
      this->y = y;
      this->z = z;
      this->formatting = formatting;
    }
    std::string text;
    float x = 0.f, y = 0.f, z = 0.f;
    TextFormatting formatting;
  };
  std::vector<StoredText> stored_text_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_QT_TEXT_RENDERER_H_
