// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/QtTextRenderer.h"

#include <GteVector.h>
#include <absl/meta/type_traits.h>

#include <QChar>
#include <QCharRef>
#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QRect>
#include <QString>
#include <QStringList>
#include <Qt>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/TranslationStack.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

namespace {

// Qt offers a QFontMetrics::horizontalAdvance to determine the width of a rendered string. This
// method is fairly slow. For rendering the text in the timers we therefore use a different method:
// We compute a lookup table storing the rendered width of all the characters and sum over all the
// characters in a string (compare GetCharacterWidthLookup, GetStringWidthFast below). The result is
// consistently a bit shorter than the correct result provided by QFontMetrics::horizontalAdvance.
// Applying the heuristic below to the result from the lookup reliably yields a fairly tight upper
// bound for the true width of the rendered string. Don't try to make sense of the formula - it is
// just a line fitted to example data.
[[nodiscard]] int MaximumHeuristic(int width, int length, uint32_t font_size) {
  return 2 + (length * static_cast<int>(font_size)) / (12 * 14) + width;
}

[[nodiscard]] float GetYOffsetFromAlignment(QtTextRenderer::VAlign alignment, float height) {
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

[[nodiscard]] float GetXOffsetFromAlignment(QtTextRenderer::HAlign alignment, float width) {
  // kOffset is a hack to compensate for subtle differences in the placement of the rendered text
  // under Linux and Windows. Setting kOffset == 0 under Windows results in texts starting left of
  // the interval border for unknown reasons (also see https://github.com/google/orbit/issues/4627).
#ifdef _WIN32
  constexpr float kOffset = 2.f;
#else
  constexpr float kOffset = 0.f;
#endif
  switch (alignment) {
    case QtTextRenderer::HAlign::Left:
      return kOffset + 0.f;
    case QtTextRenderer::HAlign::Centered:
      return kOffset - 0.5f * width;
    case QtTextRenderer::HAlign::Right:
      return kOffset - width;
    default:
      ORBIT_UNREACHABLE();
  }
}

}  // namespace

void QtTextRenderer::DrawRenderGroup(QPainter* painter, BatchRenderGroupStateManager& manager,
                                     const BatchRenderGroupId& group) {
  ORBIT_SCOPE_FUNCTION;
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  auto text_for_layer = stored_text_.find(group);
  if (text_for_layer == stored_text_.end()) {
    return;
  }
  for (const auto& text_entry : text_for_layer->second) {
    font.setPixelSize(text_entry.formatting.font_size);
    painter->setFont(font);
    painter->setPen(QColor(text_entry.formatting.color[0], text_entry.formatting.color[1],
                           text_entry.formatting.color[2], text_entry.formatting.color[3]));

    auto stencil = manager.GetGroupState(group.name).stencil;
    if (stencil.enabled) {
      Vec2i stencil_screen_pos = viewport_->WorldToScreen(Vec2(stencil.pos[0], stencil.pos[1]));
      Vec2i stencil_screen_size = viewport_->WorldToScreen(Vec2(stencil.size[0], stencil.size[1]));
      painter->setClipRect(QRect(stencil_screen_pos[0], stencil_screen_pos[1],
                                 stencil_screen_size[0], stencil_screen_size[1]));
      painter->setClipping(true);
    } else {
      painter->setClipping(false);
    }

    painter->drawText(text_entry.x, text_entry.y, text_entry.w, text_entry.h, Qt::AlignCenter,
                      text_entry.text);
  }
}

std::vector<BatchRenderGroupId> QtTextRenderer::GetRenderGroups() const {
  std::vector<BatchRenderGroupId> result;
  result.reserve(stored_text_.size());
  for (const auto& [group, unused_text] : stored_text_) {
    result.push_back(group);
  }
  return result;
};

void QtTextRenderer::AddText(const char* text, float x, float y, float z,
                             TextFormatting formatting) {
  AddText(text, x, y, z, formatting, nullptr, nullptr);
}

void QtTextRenderer::AddText(const char* text, float x, float y, float z, TextFormatting formatting,
                             Vec2* out_text_pos, Vec2* out_text_size) {
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
  const float height_entire_text = GetStringHeight(text, formatting.font_size);
  Vec2i pen_pos = viewport_->WorldToScreen(Vec2(x, y));
  LayeredVec2 transformed = translations_.TranslateXYZAndFloorXY(
      {{static_cast<float>(pen_pos[0]), static_cast<float>(pen_pos[1])}, z});

  current_render_group_.layer = transformed.z;

  const int max_width = static_cast<int>(viewport_->WorldToScreen({formatting.max_size, 0})[0]);
  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPixelSize(formatting.font_size);
  QFontMetrics metrics(font);
  float y_offset = GetYOffsetFromAlignment(formatting.valign, height_entire_text);
  const float single_line_height = GetSingleLineStringHeight(formatting.font_size);
  QStringList lines = text_as_qstring.split("\n");
  float max_line_width = 0.f;
  for (const auto& line : lines) {
    QString elided_line =
        formatting.max_size == -1.f ? line : metrics.elidedText(line, Qt::ElideRight, max_width);
    const float width = GetStringWidth(elided_line, formatting.font_size);
    max_line_width = std::max(max_line_width, width);
    const float x_offset = GetXOffsetFromAlignment(formatting.halign, width);
    stored_text_[current_render_group_].emplace_back(
        elided_line, std::lround(transformed.xy[0] + x_offset),
        std::lround(transformed.xy[1] + y_offset), std::lround(width),
        std::lround(single_line_height), formatting);
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
  // Early-out: If we can't fit a single char, there's no use to do all the expensive
  // calculations below - this is a major bottleneck in some cases
  if (formatting.max_size >= 0 && GetMinimumTextWidth(formatting.font_size) > formatting.max_size) {
    return 0.f;
  }

  const size_t text_length = std::strlen(text);
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

  const float max_width =
      formatting.max_size == -1.f ? FLT_MAX : viewport_->WorldToScreen({formatting.max_size, 0})[0];
  const QString text_as_qstring(text);
  const CharacterWidthLookup& lookup = GetCharacterWidthLookup(formatting.font_size);
  const QString trailing_text = text_as_qstring.right(trailing_chars_length);
  const float trailing_text_width = GetStringWidthFast(trailing_text, lookup, formatting.font_size);
  // If the trailing text fits we (potentially) elide the leading text.
  if (trailing_text_width < max_width) {
    const QString leading_text = text_as_qstring.left(text_length - trailing_chars_length);
    const QString elided_text =
        ElideText(leading_text, static_cast<int>(max_width - trailing_text_width), lookup,
                  formatting.font_size);
    return AddFittingSingleLineText(elided_text + trailing_text, x, y, z, formatting, lookup);
  }
  // If the trailing text doesn't fit we simply elide the entire text (the trailing text is not
  // preserved in this case).
  const QString elided_text =
      ElideText(text_as_qstring, static_cast<int>(max_width), lookup, formatting.font_size);
  if (elided_text.isEmpty()) {
    return 0.f;
  }
  return AddFittingSingleLineText(elided_text, x, y, z, formatting, lookup);
}

float QtTextRenderer::GetStringWidth(const char* text, uint32_t font_size) {
  QString text_as_qstring(text);
  return GetStringWidth(text_as_qstring, font_size);
}

float QtTextRenderer::GetStringHeight(const char* text, uint32_t font_size) {
  QString text_as_qstring(text);
  int number_of_lines = text_as_qstring.count('\n') + 1;
  return static_cast<float>(number_of_lines) * GetSingleLineStringHeight(font_size);
}

float QtTextRenderer::GetStringWidth(const QString& text, uint32_t font_size) {
  QStringList lines = text.split("\n");
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

float QtTextRenderer::GetMinimumTextWidth(uint32_t font_size) {
  auto minimum_string_width_it = minimum_string_width_cache_.find(font_size);
  if (minimum_string_width_it != minimum_string_width_cache_.end()) {
    return minimum_string_width_it->second;
  }
  // Only if we can fit one wide (hence the "W") character we start rendering text. Otherwise we
  // leave the space empty.
  constexpr char const* kMinimumString = "W";
  const float width = GetStringWidth(kMinimumString, font_size);
  minimum_string_width_cache_[font_size] = width;
  return width;
}

float QtTextRenderer::GetSingleLineStringHeight(uint32_t font_size) {
  int metrics_height = 0;
  auto it = single_line_height_cache_.find(font_size);
  if (it == single_line_height_cache_.end()) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPixelSize(static_cast<int>(font_size));
    QFontMetrics metrics(font);
    int height = metrics.height();
    metrics_height = single_line_height_cache_[font_size] = height;
  } else {
    metrics_height = it->second;
  }
  return viewport_->ScreenToWorld(Vec2i(0, metrics_height))[1];
}

const QtTextRenderer::CharacterWidthLookup& QtTextRenderer::GetCharacterWidthLookup(
    uint32_t font_size) {
  auto it = character_width_lookup_cache_.find(font_size);
  if (it == character_width_lookup_cache_.end()) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPixelSize(static_cast<int>(font_size));
    QFontMetrics metrics(font);
    CharacterWidthLookup lut;
    for (int i = 0; i < 256; i++) {
      lut[i] = metrics.horizontalAdvance(QChar(i));
    }
    it = character_width_lookup_cache_.emplace(font_size, lut).first;
  }
  return it->second;
}

float QtTextRenderer::GetStringWidthFast(const QString& text, const CharacterWidthLookup& lookup,
                                         uint32_t font_size) {
  int width = 0;
  for (const QChar& c : text) {
    width += lookup[static_cast<unsigned char>(c.toLatin1())];
  }
  const int horizontal_advance = MaximumHeuristic(width, text.length(), font_size);
  return viewport_->ScreenToWorld(Vec2i(horizontal_advance, 0))[0];
}

QString QtTextRenderer::ElideText(const QString& text, int max_width,
                                  const CharacterWidthLookup& lookup, uint32_t font_size) {
  int width_lookup = 0;
  int characters = 0;
  while (characters < text.length()) {
    const char c = text[characters].toLatin1();
    const int next_char_width = lookup[static_cast<unsigned char>(c)];
    if (MaximumHeuristic(width_lookup + next_char_width, characters, font_size) > max_width) {
      break;
    }
    width_lookup += next_char_width;
    characters++;
  }
  if (characters == text.length()) {
    return text;
  }
  QString result = text.left(characters);
  if (characters > 0) {
    result[characters - 1] = ' ';
  }
  return result;
}

float QtTextRenderer::AddFittingSingleLineText(const QString& text, float x, float y, float z,
                                               const TextFormatting& formatting,
                                               const CharacterWidthLookup& lookup) {
  const float width = GetStringWidthFast(text, lookup, formatting.font_size);
  const float single_line_height = GetSingleLineStringHeight(formatting.font_size);
  Vec2i pen_pos = viewport_->WorldToScreen(Vec2(x, y));
  LayeredVec2 transformed = translations_.TranslateXYZAndFloorXY(
      {{static_cast<float>(pen_pos[0]), static_cast<float>(pen_pos[1])}, z});
  const float x_offset = GetXOffsetFromAlignment(formatting.halign, width);
  float y_offset = GetYOffsetFromAlignment(formatting.valign, single_line_height);

  current_render_group_.layer = transformed.z;

  stored_text_[current_render_group_].emplace_back(
      text, std::lround(transformed.xy[0] + x_offset), std::lround(transformed.xy[1] + y_offset),
      std::lround(width), std::lround(single_line_height), formatting);
  return width;
}

}  // namespace orbit_gl