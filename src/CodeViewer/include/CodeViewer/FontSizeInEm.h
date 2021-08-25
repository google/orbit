// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_VIEWER_FONT_SIZE_IN_EM_H_
#define CODE_VIEWER_FONT_SIZE_IN_EM_H_

#include <QFontMetrics>

namespace orbit_code_viewer {

/*
  Strong typedef for width relative to the current font-size.
  A width given in Em scales with the font size.

  1.0 Em refers to the width of the capital letter 'M' in our implementation.
  A proper implementation should use font_metrics.pixelSize() as 1.0em, but
  as it turns out, that value is not always available. So we take a reference
  character.
  */
class FontSizeInEm {
  float value_ = 0.0f;

 public:
  constexpr explicit FontSizeInEm() = default;
  constexpr explicit FontSizeInEm(float value) : value_{value} {}

  [[nodiscard]] int ToPixels(const QFontMetrics&) const;

  [[nodiscard]] explicit operator float() const { return value_; }

  [[nodiscard]] float& Value() { return value_; }
  [[nodiscard]] float Value() const { return value_; }

  friend FontSizeInEm operator+(const FontSizeInEm& left, const FontSizeInEm& right) {
    return FontSizeInEm{left.value_ + right.value_};
  }
  friend FontSizeInEm operator-(const FontSizeInEm& left, const FontSizeInEm& right) {
    return FontSizeInEm{left.value_ - right.value_};
  }
};
}  // namespace orbit_code_viewer

#endif  // CODE_VIEWER_FONT_SIZE_IN_EM_H_