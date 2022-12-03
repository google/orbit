// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ANNOTATION_TRACK_H_
#define ORBIT_GL_ANNOTATION_TRACK_H_

#include <stdint.h>

#include <optional>
#include <string>
#include <utility>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"

class AnnotationTrack {
 public:
  [[nodiscard]] std::optional<std::pair<std::string, double>> GetWarningThreshold() const {
    return warning_threshold_;
  }
  [[nodiscard]] std::optional<std::pair<std::string, double>> GetValueUpperBound() const {
    return value_upper_bound_;
  }
  [[nodiscard]] std::optional<std::pair<std::string, double>> GetValueLowerBound() const {
    return value_lower_bound_;
  }

  void SetWarningThreshold(std::string pretty_label, double raw_value) {
    warning_threshold_ = std::make_pair(std::move(pretty_label), raw_value);
  }
  virtual void SetValueUpperBound(std::string pretty_label, double raw_value) {
    value_upper_bound_ = std::make_pair(std::move(pretty_label), raw_value);
  }
  virtual void SetValueLowerBound(std::string pretty_label, double raw_value) {
    value_lower_bound_ = std::make_pair(std::move(pretty_label), raw_value);
  }

  void DrawAnnotation(orbit_gl::PrimitiveAssembler& primitive_assembler,
                      orbit_gl::TextRenderer& text_renderer, const TimeGraphLayout* layout,
                      int indentation_level, float z);

 private:
  [[nodiscard]] virtual float GetAnnotatedTrackContentHeight() const = 0;
  [[nodiscard]] virtual Vec2 GetAnnotatedTrackPosition() const = 0;
  [[nodiscard]] virtual Vec2 GetAnnotatedTrackSize() const = 0;
  [[nodiscard]] virtual uint32_t GetAnnotationFontSize(int indentation_level) const = 0;
  [[nodiscard]] virtual std::string GetValueUpperBoundTooltip() const { return ""; }

  std::optional<std::pair<std::string, double>> warning_threshold_ = std::nullopt;
  std::optional<std::pair<std::string, double>> value_upper_bound_ = std::nullopt;
  std::optional<std::pair<std::string, double>> value_lower_bound_ = std::nullopt;
};

#endif  // ORBIT_GL_ANNOTATION_TRACK_H_