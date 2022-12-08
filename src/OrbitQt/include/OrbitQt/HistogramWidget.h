// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_HISTOGRAM_H_
#define ORBIT_HISTOGRAM_H_

#include <absl/types/span.h>
#include <stddef.h>

#include <QEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QString>
#include <QWidget>
#include <cstdint>
#include <optional>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/ScopeId.h"
#include "Statistics/Histogram.h"

namespace orbit_qt {

struct BarData {
  double frequency;
  int top_y_pos;
};

// This method returns a vector specifying the width to be drawn for each bin of a histogram
// Takes two positive intergers, returns a vector `result` of non-negative integers s.t.
// their sum equals to `histogram_width` and for all `i` and `j` `max(result[i] - result[j]) <= 1`
// `histogram_width` represents the width of the histogarm in pixels.
[[nodiscard]] std::vector<int> GenerateHistogramBinWidths(size_t number_of_bins,
                                                          int histogram_width);

// Implements a widget that draws a histogram.
// If the histogram is empty, draws a textual suggestion to select a function.
class HistogramWidget : public QWidget {
  Q_OBJECT
  using ScopeId = orbit_client_data::ScopeId;

 public:
  using QWidget::QWidget;

  void UpdateData(const std::vector<uint64_t>* data, std::string scope_name,
                  std::optional<ScopeId> scope_id);

  [[nodiscard]] QString GetTitle() const;

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

  [[nodiscard]] std::optional<orbit_statistics::HistogramSelectionRange> GetSelectionRange() const;

  void EmitSignalSelectionRangeChange() const;
  void EmitSignalTitleChange() const;
  void UpdateAndNotify();

  [[nodiscard]] bool IsOverHistogram(const QPoint& pos) const;

  struct ScopeData {
    ScopeData(const std::vector<uint64_t>* data, std::string name, ScopeId id)
        : data(data), name(std::move(name)), id(id) {}

    const std::vector<uint64_t>* data;
    std::string name;
    ScopeId id;
  };

  std::optional<ScopeData> scope_data_;

  std::stack<orbit_statistics::Histogram> histogram_stack_;
  std::stack<orbit_statistics::HistogramSelectionRange> ranges_stack_;

  struct SelectedArea {
    int selection_start_pixel;
    int selection_current_pixel;
  };

  std::optional<SelectedArea> selected_area_;

  std::optional<int> histogram_hover_x_;
};
}  // namespace orbit_qt
#endif  // ORBIT_HISTOGRAM_H_
