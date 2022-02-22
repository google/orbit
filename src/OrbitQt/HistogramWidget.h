// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_HISTOGRAM_H_
#define ORBIT_HISTOGRAM_H_

#include <absl/types/span.h>

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <cstdint>
#include <optional>
#include <stack>
#include <string>

#include "Statistics/Histogram.h"

// Implements a widget that draws a histogram.
// If the histogram is empty, draws a textual suggestion to select a function.
class HistogramWidget : public QWidget {
 public:
  using QWidget::QWidget;

  void UpdateData(std::vector<uint64_t> data, std::string function_name);

 protected:
  void paintEvent(QPaintEvent* /*event*/) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  [[nodiscard]] bool IsSelectionActive() const;

  [[nodiscard]] uint64_t MinValue() const;
  [[nodiscard]] uint64_t MaxValue() const;

  [[nodiscard]] int Width() const;
  [[nodiscard]] int Height() const;
  [[nodiscard]] int WidthMargin() const;
  [[nodiscard]] int HeightMargin() const;

  std::optional<const std::vector<uint64_t>> data_;
  std::optional<const std::string> function_name_;

  std::stack<orbit_statistics::Histogram> histogram_stack_;

  std::optional<int> selection_start_pixel;
  std::optional<int> selection_current_pixel;
};

#endif  // ORBIT_HISTOGRAM_H_
