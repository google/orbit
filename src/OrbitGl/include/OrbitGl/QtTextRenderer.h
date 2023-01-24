// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_QT_TEXT_RENDERER_H_
#define ORBIT_GL_QT_TEXT_RENDERER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>

#include <QPainter>
#include <QString>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TextRendererInterface.h"

namespace orbit_gl {

// Qt implementation of TextRenderer.
class QtTextRenderer : public TextRenderer {
 public:
  void Init() override{};
  void Clear() override {
    stored_text_.clear();
    current_render_group_ = BatchRenderGroupId();
  };

  [[nodiscard]] std::vector<BatchRenderGroupId> GetRenderGroups() const override;
  void DrawRenderGroup(QPainter* painter, BatchRenderGroupStateManager& manager,
                       const BatchRenderGroupId& group) override;

  void AddText(const char* text, float x, float y, float z, TextFormatting formatting) override;
  void AddText(const char* text, float x, float y, float z, TextFormatting formatting,
               Vec2* out_text_pos, Vec2* out_text_size) override;

  float AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                        TextFormatting formatting,
                                        size_t trailing_chars_length) override;

  [[nodiscard]] float GetStringWidth(const char* text, uint32_t font_size) override;
  [[nodiscard]] float GetStringHeight(const char* text, uint32_t font_size) override;
  [[nodiscard]] float GetMinimumTextWidth(uint32_t font_size) override;

 private:
  using CharacterWidthLookup = std::array<int, 256>;

  [[nodiscard]] float GetStringWidth(const QString& text, uint32_t font_size);
  [[nodiscard]] float GetSingleLineStringHeight(uint32_t font_size);
  [[nodiscard]] const CharacterWidthLookup& GetCharacterWidthLookup(uint32_t font_size);
  [[nodiscard]] float GetStringWidthFast(const QString& text, const CharacterWidthLookup& lookup,
                                         uint32_t font_size);
  [[nodiscard]] static QString ElideText(const QString& text, int max_width,
                                         const CharacterWidthLookup& lookup, uint32_t font_size);
  [[nodiscard]] float AddFittingSingleLineText(const QString& text, float x, float y, float z,
                                               const TextFormatting& formatting,
                                               const CharacterWidthLookup& lookup);
  struct StoredText {
    StoredText() = default;
    StoredText(const QString& text, int x, int y, int w, int h, TextFormatting formatting)
        : text(text), x(x), y(y), w(w), h(h), formatting(formatting) {}
    QString text;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    TextFormatting formatting;
  };
  absl::flat_hash_map<BatchRenderGroupId, std::vector<StoredText>> stored_text_;
  absl::flat_hash_map<uint32_t, float> minimum_string_width_cache_;
  absl::flat_hash_map<uint32_t, CharacterWidthLookup> character_width_lookup_cache_;
  absl::flat_hash_map<uint32_t, int> single_line_height_cache_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_QT_TEXT_RENDERER_H_
