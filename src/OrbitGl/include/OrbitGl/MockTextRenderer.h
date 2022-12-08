// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MOCK_TEXT_RENDERER_H_
#define ORBIT_GL_MOCK_TEXT_RENDERER_H_

#include <gmock/gmock.h>
#include <stddef.h>
#include <stdint.h>

#include <QPainter>
#include <algorithm>
#include <set>
#include <vector>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/TextRenderer.h"

namespace orbit_gl {

class MockTextRenderer : public TextRenderer {
 public:
  explicit MockTextRenderer();

  MOCK_METHOD(void, Init, (), (override));
  void Clear() override;

  MOCK_METHOD(void, RenderLayer, (QPainter*, float), (override));
  [[nodiscard]] std::vector<float> GetLayers() const override {
    return std::vector<float>(z_layers_.begin(), z_layers_.end());
  }

  void AddText(const char* text, float x, float y, float z, TextFormatting formatting) override;
  void AddText(const char* text, float x, float y, float z, TextFormatting formatting,
               Vec2* out_text_pos, Vec2* out_text_size) override;

  float AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                        TextFormatting formatting,
                                        size_t trailing_chars_length) override;

  [[nodiscard]] float GetStringWidth(const char* text, uint32_t font_size) override;
  [[nodiscard]] float GetStringHeight(const char* text, uint32_t font_size) override;

  [[nodiscard]] bool HasAddTextsSameLength() const {
    return num_characters_in_add_text_.size() <= 1;
  }
  [[nodiscard]] bool AreAddTextsAlignedVertically() const {
    return vertical_position_in_add_text.size() <= 1;
  }
  [[nodiscard]] const int GetNumAddTextCalls() const { return num_add_text_calls_; }
  [[nodiscard]] bool IsTextInsideRectangle(const Vec2& start, const Vec2& size) const;
  [[nodiscard]] bool IsTextBetweenZLayers(float z_layer_min, float z_layer_max) const;

 private:
  void AdjustDrawingBoundaries(Vec2 point);

  Vec2 min_point_;
  Vec2 max_point_;
  std::set<float> z_layers_;
  std::set<uint32_t> num_characters_in_add_text_;
  std::set<float> vertical_position_in_add_text;
  int num_add_text_calls_ = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MOCK_TEXT_RENDERER_H_
