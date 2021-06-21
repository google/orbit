// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ANNOTATION_TRACK_H_
#define ORBIT_GL_ANNOTATION_TRACK_H_

#include <optional>
#include <string>

#include "Batcher.h"
#include "CoreMath.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"

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

  void SetWarningThreshold(const std::string& pretty_label, double raw_value) {
    warning_threshold_ = std::make_pair(pretty_label, raw_value);
  }
  virtual void SetValueUpperBound(const std::string& pretty_label, double raw_value) {
    value_upper_bound_ = std::make_pair(pretty_label, raw_value);
  }
  virtual void SetValueLowerBound(const std::string& pretty_label, double raw_value) {
    value_lower_bound_ = std::make_pair(pretty_label, raw_value);
  }

  void DrawAnnotation(Batcher& batcher, TextRenderer& text_renderer, TimeGraphLayout* layout,
                      float z);

 private:
  [[nodiscard]] virtual float GetAnnotatedTrackContentHeight() const = 0;
  [[nodiscard]] virtual Vec2 GetAnnotatedTrackPosition() const = 0;
  [[nodiscard]] virtual Vec2 GetAnnotatedTrackSize() const = 0;

  std::optional<std::pair<std::string, double>> warning_threshold_ = std::nullopt;
  std::optional<std::pair<std::string, double>> value_upper_bound_ = std::nullopt;
  std::optional<std::pair<std::string, double>> value_lower_bound_ = std::nullopt;
};

#endif  // ORBIT_GL_ANNOTATION_TRACK_H_