// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "QtTextRenderer.h"

#include <absl/strings/str_split.h>

#include <QColor>
#include <QFontDatabase>
#include <QPainter>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <string>
#include <utility>

#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"

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
      // TextRendererInterface. One would expect returning -height here but that would mean we would
      // need to alter call sites of AddText.
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
                      QString(text_entry.text.c_str()));
  }
}

void QtTextRenderer::RenderDebug(PrimitiveAssembler* /*primitive_assembler*/) {}

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
  AddText(text, x, y, z, formatting, nullptr, nullptr);
}

void QtTextRenderer::AddText(const char* text, float x, float y, float z, TextFormatting formatting,
                             Vec2* out_text_pos, Vec2* out_text_size) {
  const size_t text_length = strlen(text);
  if (text_length == 0) {
    return;
  }
  Vec2i pen_pos = viewport_->WorldToScreen(Vec2(x, y));
  LayeredVec2 transformed = translations_.TranslateXYZAndFloorXY(
      {{static_cast<float>(pen_pos[0]), static_cast<float>(pen_pos[1])}, z});
  const float max_width =
      formatting.max_size == -1.f ? FLT_MAX : viewport_->WorldToScreen({formatting.max_size, 0})[0];
  // Find out how many characters from text can fit into max_width via a binary search.
  std::string text_as_string(text);
  size_t idx_min = 1;
  size_t idx_max = text_length;
  if (GetStringWidth(text_as_string.substr(0, 1).c_str(), formatting.font_size) > max_width) {
    return;
  }
  if (GetStringWidth(text, formatting.font_size) <= max_width) {
    idx_min = text_length;
  }
  while (idx_max - idx_min > 1) {
    const size_t candidate_idx = (idx_min + idx_max) / 2;
    const std::string candidate_string = text_as_string.substr(0, candidate_idx);
    if (GetStringWidth(candidate_string.c_str(), formatting.font_size) > max_width) {
      idx_max = candidate_idx;
    } else {
      idx_min = candidate_idx;
    }
  }
  text_as_string = text_as_string.substr(0, idx_min);
  float y_offset = GetYOffsetFromAlignment(
      formatting.valign, GetStringHeight(text_as_string.c_str(), formatting.font_size));
  const float single_line_height = GetStringHeight(".", formatting.font_size);
  const std::vector<std::string> lines = absl::StrSplit(text_as_string, '\n');
  for (const auto& line : lines) {
    const float width = GetStringWidth(line.c_str(), formatting.font_size);
    const float x_offset = GetXOffsetFromAlignment(formatting.halign, width);
    stored_text_[transformed.z].emplace_back(
        line.c_str(), std::lround(transformed.xy[0] + x_offset),
        std::lround(transformed.xy[1] + y_offset), std::lround(width),
        std::lround(single_line_height), formatting);
    y_offset += single_line_height;
  }

  if (out_text_pos != nullptr) {
    (*out_text_pos)[0] =
        transformed.xy[0] +
        GetXOffsetFromAlignment(formatting.halign, GetStringWidth(text, formatting.font_size));
    (*out_text_pos)[1] =
        transformed.xy[1] +
        GetYOffsetFromAlignment(formatting.valign, GetStringHeight(text, formatting.font_size));
  }
  if (out_text_size != nullptr) {
    *out_text_size = Vec2(GetStringWidth(text, formatting.font_size),
                          GetStringHeight(text, formatting.font_size));
  }
}

float QtTextRenderer::AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                                      TextFormatting formatting,
                                                      size_t trailing_chars_length) {
  const size_t text_length = strlen(text);
  if (text_length < trailing_chars_length) {
    ORBIT_ERROR(
        "Trailing character length was longer than the string itself. text: \"%s\" "
        "trailing_chars_length: %d",
        text, trailing_chars_length);
    trailing_chars_length = text_length;
  }

  if (text_length == 0) {
    return 0.f;
  }
  // Early-out: If we can't fit a single char, there's no use to do all the expensive
  // calculations below - this is a major bottleneck in some cases
  if (formatting.max_size >= 0 && GetStringWidth(".", formatting.font_size) > formatting.max_size) {
    return 0.f;
  }

  // Test if the entire string fits.
  const float max_width =
      formatting.max_size == -1.f ? FLT_MAX : viewport_->WorldToScreen({formatting.max_size, 0})[0];
  const float entire_width = GetStringWidth(text, formatting.font_size);
  if (entire_width <= max_width) {
    AddText(text, x, y, z, formatting);
    return entire_width;
  }
  // The entire string does not fit. We try to fit something of the form
  // leading_text + "... " + trailing_text
  // where leading_text is a variable amount of characters from the beginning of the string and
  // trailing_text is specified by the trailing_chars_length parameter.
  const std::string text_as_string(text);
  const std::string trailing_text = text_as_string.substr(text_length - trailing_chars_length);
  const std::string leading_text = text_as_string.substr(0, text_length - trailing_chars_length);
  const size_t leading_length = leading_text.length();
  const std::string ellipsis_plus_trailing_text = std::string("... ") + trailing_text;
  size_t fitting_chars_count = 1;
  std::string candidate_string =
      leading_text.substr(0, fitting_chars_count).append(ellipsis_plus_trailing_text);

  // Test if we can fit the minimal ellipsised string.
  if (GetStringWidth(candidate_string.c_str(), formatting.font_size) > max_width) {
    // We can't fit any ellipsised string: Even with one leading character the thing becomes too
    // long. So we let AddText truncate the entire string.
    Vec2 dims;
    AddText(text, x, y, z, formatting, nullptr, &dims);
    return dims[0];
  }

  // Binary search between 1 and leading_length (we know 1 works and leading_length does not).
  size_t idx_min = 1;
  size_t idx_max = leading_length;
  while (idx_max - idx_min > 1) {
    size_t candidate_idx = (idx_min + idx_max) / 2;
    candidate_string = leading_text.substr(0, candidate_idx).append(ellipsis_plus_trailing_text);
    if (GetStringWidth(candidate_string.c_str(), formatting.font_size) > max_width) {
      idx_max = candidate_idx;
    } else {
      idx_min = candidate_idx;
    }
  }
  fitting_chars_count = idx_min;
  candidate_string =
      leading_text.substr(0, fitting_chars_count).append(ellipsis_plus_trailing_text);
  AddText(candidate_string.c_str(), x, y, z, formatting);
  return GetStringWidth(candidate_string.c_str(), formatting.font_size);
}

float QtTextRenderer::GetStringWidth(const char* text, uint32_t font_size) {
  const std::vector<std::string> lines = absl::StrSplit(std::string(text), '\n');
  float max_width = 0.f;
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPixelSize(static_cast<int>(font_size));
  QFontMetrics metrics(font);
  for (const std::string& line : lines) {
    max_width = std::max(
        max_width, viewport_->ScreenToWorld(Vec2i(metrics.horizontalAdvance(line.c_str()), 0))[0]);
  }
  return max_width;
}

float QtTextRenderer::GetStringHeight(const char* text, uint32_t font_size) {
  const float number_of_lines = static_cast<float>(std::count(text, text + strlen(text), '\n') + 1);
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPixelSize(static_cast<int>(font_size));
  QFontMetrics metrics(font);
  return number_of_lines * viewport_->ScreenToWorld(Vec2i(0, metrics.height()))[1];
}

}  // namespace orbit_gl