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
#include <vector>

#include "App.h"
#include "Statistics/Histogram.h"

// Implements a widget that draws a histogram.
// If the histogram is empty, draws a textual suggestion to select a function.
class HistogramWidget : public QWidget {
  Q_OBJECT

 public:
  using QWidget::QWidget;

  void UpdateData(const std::vector<uint64_t>* data, std::string function_name,
                  uint64_t function_id);
  [[nodiscard]] QString GetDefaultTitleMessage() const;

 signals:
  void SignalSelectionRangeChange(std::optional<orbit_statistics::HistogramSelectionRange>) const;
  void SignalTitleChange(QString) const;

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

  [[nodiscard]] QString GetTitle() const;

  [[nodiscard]] std::optional<orbit_statistics::HistogramSelectionRange> GetSelectionRange() const;
  void EmitSignalSelectionRangeChange() const;

  struct FunctionData {
    FunctionData(const std::vector<uint64_t>* data, std::string name, uint64_t id)
        : data(data), name(std::move(name)), id(id) {}

    const std::vector<uint64_t>* data;
    std::string name;
    uint64_t id;
  };

  std::optional<FunctionData> function_data_;

  std::stack<orbit_statistics::Histogram> histogram_stack_;
  std::stack<orbit_statistics::HistogramSelectionRange> ranges_stack_;

  struct SelectedArea {
    int selection_start_pixel;
    int selection_current_pixel;
  };

  std::optional<SelectedArea> selected_area_;
};

#endif  // ORBIT_HISTOGRAM_H_
