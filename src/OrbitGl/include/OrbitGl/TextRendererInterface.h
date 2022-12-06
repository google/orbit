// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TEXT_RENDERER_INTERFACE_H_
#define ORBIT_GL_TEXT_RENDERER_INTERFACE_H_

#include <GteVector.h>
#include <stdint.h>

#include <vector>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "QPainter"

namespace orbit_gl {

class TextRendererInterface {
 public:
  enum class HAlign { Left, Right, Centered };
  enum class VAlign { Top, Middle, Bottom };

  struct TextFormatting {
    uint32_t font_size = 14;
    Color color = Color(255, 255, 255, 255);
    float max_size = -1.f;
    HAlign halign = HAlign::Left;
    VAlign valign = VAlign::Top;
  };
  virtual ~TextRendererInterface() = default;

  virtual void Init() = 0;
  virtual void Clear() = 0;

  [[nodiscard]] virtual std::vector<BatchRenderGroupId> GetRenderGroups() const = 0;
  virtual void DrawRenderGroup(QPainter* painter, const BatchRenderGroupId& group) = 0;

  [[nodiscard]] virtual BatchRenderGroupId GetCurrentRenderGroup() const = 0;
  virtual void SetCurrentRenderGroup(const BatchRenderGroupId& render_group) = 0;

  // Add a - potentially multiline - text at the given position and z-layer and with the specifier
  // formatting. If formatting.max_size is set all lines are elided to fit into this width.
  virtual void AddText(const char* text, float x, float y, float z, TextFormatting formatting) = 0;
  virtual void AddText(const char* text, float x, float y, float z, TextFormatting formatting,
                       Vec2* out_text_pos, Vec2* out_text_size) = 0;

  // Add a single line of text at the given position and z-layer and with the specifier formatting.
  // The renderer will shorten the text if the width exceeds formatting.max_size. The shortening
  // will happen in a way that tries to preserve the given numnber of trailing characters.
  // This is mainly used to preserve the duration in the text of time intervals. E.g. something like
  // "MyVeryLongButNotSoImportantMethodName 2.35 ms" will render as "MyVery...2.35 ms".
  virtual float AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                                TextFormatting formatting,
                                                size_t trailing_chars_length) = 0;

  [[nodiscard]] virtual float GetStringWidth(const char* text, uint32_t font_size) = 0;
  [[nodiscard]] virtual float GetStringHeight(const char* text, uint32_t font_size) = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TEXT_RENDERER_INTERFACE_H_
