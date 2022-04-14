// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MockTextRenderer.h"

#include "CoreMath.h"

namespace orbit_gl {

// Clearing counters also in creation to not duplicate code.
MockTextRenderer::MockTextRenderer() { Clear(); }

void MockTextRenderer::Clear() {
  min_point_ = Vec3{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max()};
  max_point_ = Vec3{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::lowest()};
  num_characters_in_add_text_.clear();
  vertical_position_in_add_text.clear();
  num_add_text_calls_ = 0;
}

void MockTextRenderer::AddText(const char* text, float x, float y, float z,
                               TextFormatting formatting) {
  return AddText(text, x, y, z, formatting, nullptr, nullptr);
}

void MockTextRenderer::AddText(const char* text, float x, float y, float z,
                               TextFormatting formatting, Vec2* out_text_pos, Vec2* out_text_size) {
  float text_width = GetStringWidth(text, formatting.font_size);
  float text_height = GetStringHeight(text, formatting.font_size);

  float real_start_x = formatting.halign == HAlign::Left ? x : x - text_width;
  float real_start_y;
  switch (formatting.valign) {
    case VAlign::Top:
      real_start_y = y;
      break;
    case VAlign::Middle:
      real_start_y = y - text_height / 2.f;
      break;
    case VAlign::Bottom:
      real_start_y = y - text_height;
      break;
  }

  UpdateDrawingBoundaries({real_start_x, real_start_y, z});
  UpdateDrawingBoundaries({real_start_x + text_width, real_start_y + text_height, z});
  num_add_text_calls_++;
  num_characters_in_add_text_.insert(strlen(text));
  vertical_position_in_add_text.insert(real_start_y);

  if (out_text_pos != nullptr) {
    *out_text_pos = {real_start_x, real_start_y};
  }
  if (out_text_size != nullptr) {
    *out_text_size = {text_width, text_height};
  }
}

float MockTextRenderer::AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                                        TextFormatting formatting,
                                                        size_t /*trailing_chars_length*/) {
  AddText(text, x, y, z, formatting);
  return GetStringWidth(text, formatting.font_size);
}

// GetStringWidth is being a bit over-estimated when using font_size. Anyway, the estimation is
// really close to the real width of the widest character ('W').
float MockTextRenderer::GetStringWidth(const char* text, uint32_t font_size) {
  return strlen(text) * font_size;
};

// GetStringHeight is clearly over-estimated here. Comparting to the one in OpenGlTextRenderer, the
// real height is 10 (with font-size 14) and 8 (with font-size 10). It should not be a problem.
float MockTextRenderer::GetStringHeight(const char* /*text*/, uint32_t font_size) {
  return font_size;
}

[[nodiscard]] bool MockTextRenderer::IsTextInsideRectangle(Vec2 start, Vec2 size) const {
  return IsInsideRectangle(Vec3ToVec2(min_point_), start, size) &&
         IsInsideRectangle(Vec3ToVec2(max_point_), start, size);
}

bool MockTextRenderer::IsTextBetweenZLayers(float z_layer_min, float z_layer_max) const {
  return IsElementOf(min_point_[2], ClosedInterval::FromValues(z_layer_min, z_layer_max)) &&
         IsElementOf(max_point_[2], ClosedInterval::FromValues(z_layer_min, z_layer_max));
}

void MockTextRenderer::UpdateDrawingBoundaries(Vec3 point) {
  min_point_[0] = std::min(point[0], min_point_[0]);
  min_point_[1] = std::min(point[1], min_point_[1]);
  min_point_[2] = std::min(point[2], min_point_[2]);
  max_point_[0] = std::max(point[0], max_point_[0]);
  max_point_[1] = std::max(point[1], max_point_[1]);
  max_point_[2] = std::max(point[2], max_point_[2]);
}

}  // namespace orbit_gl
