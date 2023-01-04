// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/QtTextRenderer.h"

#include <GteVector.h>
#include <absl/meta/type_traits.h>

#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QString>
#include <QStringList>
#include <Qt>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <string>

#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/TranslationStack.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

namespace {

float GetYOffsetFromAlignment(QtTextRenderer::VAlign alignment, float height) {
  switch (alignment) {
    case QtTextRenderer::VAlign::Top:
      return 0.f;
    case QtTextRenderer::VAlign::Middle:
      return -0.5f * height;
    case QtTextRenderer::VAlign::Bottom:
      // This is a hack to match the behaviour of a previous implementation of
      // TextRendererInterface. A previous implementation returned the height of the actual
      // rendered glyphs. The new implementation returns the maximum heigth of a rendered line of
      // text (plus potentially some margin - not sure about that). One would expect returning
      // -height here but that would mean we would need to alter call sites of AddText.
      return (-5.f / 6.f) * height;
    default:
      ORBIT_UNREACHABLE();
  }
}

float GetXOffsetFromAlignment(QtTextRenderer::HAlign alignment, float width) {
  switch (alignment) {
    case QtTextRenderer::HAlign::Left:
      return 0.f;
    case QtTextRenderer::HAlign::Centered:
      return -0.5f * width;
    case QtTextRenderer::HAlign::Right:
      return -width;
    default:
      ORBIT_UNREACHABLE();
  }
}

}  // namespace

void QtTextRenderer::RenderLayer(QPainter* painter, float layer) {
  ORBIT_SCOPE_FUNCTION;
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  auto text_for_layer = stored_text_.find(layer);
  if (text_for_layer == stored_text_.end()) {
    return;
  }
  for (const auto& text_entry : text_for_layer->second) {
    font.setPixelSize(text_entry.formatting.font_size);
    painter->setFont(font);
    painter->setPen(QColor(text_entry.formatting.color[0], text_entry.formatting.color[1],
                           text_entry.formatting.color[2], text_entry.formatting.color[3]));
    painter->drawText(text_entry.x, text_entry.y, text_entry.w, text_entry.h, Qt::AlignCenter,
                      text_entry.text);
  }
}

std::vector<float> QtTextRenderer::GetLayers() const {
  std::vector<float> result(stored_text_.size());
  int index = 0;
  for (const auto& [layer_z, unused_text] : stored_text_) {
    result[index++] = layer_z;
  }
  std::sort(result.begin(), result.end());
  return result;
};

void QtTextRenderer::AddText(const char* text, float x, float y, float z,
                             TextFormatting formatting) {
  ORBIT_SCOPE_FUNCTION;
  AddText(text, x, y, z, formatting, nullptr, nullptr);
}

void QtTextRenderer::AddText(const char* text, float x, float y, float z, TextFormatting formatting,
                             Vec2* out_text_pos, Vec2* out_text_size) {
  ORBIT_SCOPE_FUNCTION;
  if (out_text_pos != nullptr) {
    (*out_text_pos)[0] = (*out_text_pos)[1] = 0.f;
  }
  if (out_text_size != nullptr) {
    (*out_text_size)[0] = (*out_text_size)[1] = 0.f;
  }
  QString text_as_qstring(text);
  const size_t text_length = text_as_qstring.length();
  if (text_length == 0) {
    return;
  }
  float max_line_width = 0.f;
  const float height_entire_text = GetStringHeight(text, formatting.font_size);
  Vec2i pen_pos = viewport_->WorldToScreen(Vec2(x, y));
  LayeredVec2 transformed = translations_.TranslateXYZAndFloorXY(
      {{static_cast<float>(pen_pos[0]), static_cast<float>(pen_pos[1])}, z});
  const int max_width = static_cast<int>(viewport_->WorldToScreen({formatting.max_size, 0})[0]);
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPixelSize(formatting.font_size);
  QFontMetrics metrics(font);
  float y_offset = GetYOffsetFromAlignment(formatting.valign, height_entire_text);
  const float single_line_height = GetStringHeight(".", formatting.font_size);
  QStringList lines = text_as_qstring.split("\n");
  for (const auto& line : lines) {
    QString elided_line =
        formatting.max_size == -1.f ? line : metrics.elidedText(line, Qt::ElideRight, max_width);
    const float width = GetStringWidth(elided_line, formatting.font_size);
    max_line_width = std::max(max_line_width, width);
    const float x_offset = GetXOffsetFromAlignment(formatting.halign, width);
    stored_text_[transformed.z].emplace_back(elided_line, std::lround(transformed.xy[0] + x_offset),
                                             std::lround(transformed.xy[1] + y_offset),
                                             std::lround(width), std::lround(single_line_height),
                                             formatting);
    y_offset += single_line_height;
  }

  if (out_text_pos != nullptr) {
    (*out_text_pos)[0] =
        transformed.xy[0] + GetXOffsetFromAlignment(formatting.halign, max_line_width);
    (*out_text_pos)[1] =
        transformed.xy[1] + GetYOffsetFromAlignment(formatting.valign, height_entire_text);
  }
  if (out_text_size != nullptr) {
    *out_text_size = Vec2(max_line_width, height_entire_text);
  }
}

float QtTextRenderer::AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                                      TextFormatting formatting,
                                                      size_t trailing_chars_length) {
  ORBIT_SCOPE_FUNCTION;
  QString text_as_qstring(text);
  const size_t text_length = text_as_qstring.length();
  if (text_length == 0) {
    return 0.f;
  }
  if (text_length < trailing_chars_length) {
    ORBIT_ERROR(
        "Trailing character length was longer than the string itself. text: \"%s\" "
        "trailing_chars_length: %d",
        text, trailing_chars_length);
    trailing_chars_length = text_length;
  }
  // Early-out: If we can't fit a single char, there's no use to do all the expensive
  // calculations below - this is a major bottleneck in some cases
  if (formatting.max_size >= 0 && GetMinimumTextWidth(formatting.font_size) > formatting.max_size) {
    return 0.f;
  }

  const float max_width =
      formatting.max_size == -1.f ? FLT_MAX : viewport_->WorldToScreen({formatting.max_size, 0})[0];
  const QString trailing_text = text_as_qstring.right(trailing_chars_length);
  const QString leading_text = text_as_qstring.left(text_length - trailing_chars_length);
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPixelSize(formatting.font_size);
  QFontMetrics metrics(font);
  const float trailing_text_width = GetStringWidth(trailing_text, formatting.font_size);
  QString elided_text;
  if (trailing_text_width < max_width) {
    elided_text = metrics.elidedText(leading_text, Qt::ElideRight,
                                     static_cast<int>(max_width - trailing_text_width));
  }
  if (elided_text.isEmpty()) {
    // We can't fit any elided string with the trailing characters preserved so we elide the entire
    // string and accept that the trailing characters are truncated.
    elided_text = metrics.elidedText(text_as_qstring, Qt::ElideRight, static_cast<int>(max_width));
    if (elided_text.isEmpty()) {
      return 0.f;
    }
    return AddFittingSingleLineText(elided_text, x, y, z, formatting);
  }
  return AddFittingSingleLineText(elided_text + trailing_text, x, y, z, formatting);
}

float QtTextRenderer::GetStringWidth(const char* text, uint32_t font_size) {
  QString text_as_qstring(text);
  QStringList lines = text_as_qstring.split("\n");
  float max_width = 0.f;
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPixelSize(static_cast<int>(font_size));
  QFontMetrics metrics(font);
  for (const QString& line : lines) {
    max_width =
        std::max(max_width, viewport_->ScreenToWorld(Vec2i(metrics.horizontalAdvance(line), 0))[0]);
  }
  return max_width;
}

float QtTextRenderer::GetStringHeight(const char* text, uint32_t font_size) {
  QString text_as_qstring(text);
  QStringList lines = text_as_qstring.split("\n");
  const float number_of_lines = lines.size();
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPixelSize(static_cast<int>(font_size));
  QFontMetrics metrics(font);
  return number_of_lines * viewport_->ScreenToWorld(Vec2i(0, metrics.height()))[1];
}

float QtTextRenderer::GetStringWidth(const QString& text, uint32_t font_size) {
  std::string text_as_string = text.toStdString();
  return GetStringWidth(text_as_string.c_str(), font_size);
}

float QtTextRenderer::GetMinimumTextWidth(uint32_t font_size) {
  static absl::flat_hash_map<uint32_t, float> minimum_string_width_cache;
  auto minimum_string_width_it = minimum_string_width_cache.find(font_size);
  if (minimum_string_width_it != minimum_string_width_cache.end()) {
    return minimum_string_width_it->second;
  }
  // Only if we can fit one wide (hence the "W") character plus the ellipsis dots we start rendering
  // text. Otherwise we leave the space empty.
  constexpr char const* kMinimumString = "W...";
  const float width = GetStringWidth(kMinimumString, font_size);
  minimum_string_width_cache[font_size] = width;
  return width;
}

float QtTextRenderer::AddFittingSingleLineText(const QString& text, float x, float y, float z,
                                               TextFormatting formatting) {
  const float width = GetStringWidth(text, formatting.font_size);
  const float single_line_height = GetStringHeight(".", formatting.font_size);
  Vec2i pen_pos = viewport_->WorldToScreen(Vec2(x, y));
  LayeredVec2 transformed = translations_.TranslateXYZAndFloorXY(
      {{static_cast<float>(pen_pos[0]), static_cast<float>(pen_pos[1])}, z});
  const float x_offset = GetXOffsetFromAlignment(formatting.halign, width);
  float y_offset = GetYOffsetFromAlignment(formatting.valign, single_line_height);
  stored_text_[transformed.z].emplace_back(
      text, std::lround(transformed.xy[0] + x_offset), std::lround(transformed.xy[1] + y_offset),
      std::lround(width), std::lround(single_line_height), formatting);
  return width;
}

}  // namespace orbit_gl