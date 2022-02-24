// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "HistogramWidget.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <qnamespace.h>

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>

#include "DisplayFormats/DisplayFormats.h"
#include "Statistics/Histogram.h"

constexpr double kRelativeMargin = 0.1;

constexpr uint32_t kVerticalTickCount = 3;
constexpr uint32_t kHorizontalTickCount = 3;
constexpr int kTickLength = 5;

const QColor kSelectionColor = QColor(128, 128, 255, 128);

[[nodiscard]] static int RoundToClosestInt(double x) { return static_cast<int>(std::round(x)); }

// if `length > 0`, the line will be plot to the right from `start` and to the left otherwise
static void DrawHorizontalLine(QPainter& painter, const QPoint& start, int length) {
  painter.drawLine(start, {start.x() + length, start.y()});
}

// if `length > 0`, the line will be plot downwards from `start` and upwards otherwise
static void DrawVerticalLine(QPainter& painter, const QPoint& start, int length) {
  painter.drawLine(start, {start.x(), start.y() + length});
}

static void DrawHorizontalAxis(QPainter& painter, const QPoint& axes_intersection,
                               const orbit_statistics::Histogram& histogram, int length,
                               uint64_t min_value) {
  DrawHorizontalLine(painter, axes_intersection, length);

  const auto tick_spacing_as_value = (histogram.max - min_value) / kHorizontalTickCount;
  const int tick_spacing_pixels =
      RoundToClosestInt(static_cast<double>(length) / kHorizontalTickCount);

  int current_tick_location = axes_intersection.x();
  uint64_t current_tick_value = min_value;

  const QFontMetrics font_metrics(painter.font());

  for (uint32_t i = 0; i <= kHorizontalTickCount; ++i) {
    DrawVerticalLine(painter, {current_tick_location, axes_intersection.y()}, kTickLength);

    const QString tick_label = QString::fromStdString(
        orbit_display_formats::GetDisplayTime(absl::Nanoseconds(current_tick_value)));
    const QRect tick_label_bounding_rect = font_metrics.boundingRect(tick_label);
    painter.drawText(current_tick_location - tick_label_bounding_rect.width() / 2,
                     axes_intersection.y() + kTickLength + tick_label_bounding_rect.height(),
                     tick_label);

    current_tick_location += tick_spacing_pixels;
    current_tick_value += tick_spacing_as_value;
  }
}

static void DrawVerticalAxis(QPainter& painter, const QPoint& axes_intersection, int length,
                             double max_freq) {
  DrawVerticalLine(painter, axes_intersection, -length);

  const double tick_spacing_as_value = max_freq / kVerticalTickCount;
  const int tick_spacing_pixels =
      RoundToClosestInt(static_cast<double>(length) / kVerticalTickCount);

  double current_tick_value = tick_spacing_as_value;
  int current_tick_location = axes_intersection.y() - tick_spacing_pixels;

  const QFontMetrics font_metrics(painter.font());

  for (uint32_t i = 1; i <= kVerticalTickCount; ++i) {
    DrawHorizontalLine(painter, {axes_intersection.x(), current_tick_location}, -kTickLength);

    QString tick_label = QString::fromStdString(absl::StrFormat("%.2f", current_tick_value));

    QRect tick_label_bounding_rect = font_metrics.boundingRect(tick_label);
    painter.drawText(axes_intersection.x() - tick_label_bounding_rect.width() - kTickLength,
                     current_tick_location + tick_label_bounding_rect.height() / 2, tick_label);

    current_tick_location -= tick_spacing_pixels;
    current_tick_value += tick_spacing_as_value;
  }
}

[[nodiscard]] static int ValueToAxisLocation(double value, int axis_length, double min_value,
                                             double max_value) {
  if (min_value == max_value) max_value++;
  return RoundToClosestInt(((value - min_value) / (max_value - min_value)) * axis_length);
}

[[nodiscard]] static double GetFreq(const orbit_statistics::Histogram& histogram, size_t i) {
  return static_cast<double>(histogram.counts[i]) / static_cast<double>(histogram.data_set_size);
}

static void DrawHistogram(QPainter& painter, const QPoint& axes_intersection,
                          const orbit_statistics::Histogram& histogram, int horizontal_axis_length,
                          int vertical_axis_length, double max_freq, int vertical_shift,
                          uint64_t min_value) {
  for (size_t i = 0; i < histogram.counts.size(); ++i) {
    const uint64_t bin_from = histogram.min + i * histogram.bin_width;
    const uint64_t bin_to = bin_from + histogram.bin_width;

    double freq = GetFreq(histogram, i);
    if (freq > 0) {
      const QPoint top_left(
          axes_intersection.x() +
              ValueToAxisLocation(bin_from, horizontal_axis_length, min_value, histogram.max),
          axes_intersection.y() - vertical_shift -
              ValueToAxisLocation(freq, vertical_axis_length, 0, max_freq));
      const QPoint lower_right(
          axes_intersection.x() +
              ValueToAxisLocation(bin_to, horizontal_axis_length, min_value, histogram.max),
          axes_intersection.y() - vertical_shift);
      const QRect bar(top_left, lower_right);
      painter.fillRect(bar, Qt::cyan);
    }
  }
}

void HistogramWidget::UpdateData(const std::vector<uint64_t>* data, std::string function_name) {
  histogram_stack_ = {};

  function_data_.emplace(data, std::move(function_name));

  if (data != nullptr) {
    std::optional<orbit_statistics::Histogram> histogram =
        orbit_statistics::BuildHistogram(*function_data_->data);
    if (histogram) {
      histogram_stack_.push(std::move(*histogram));
    }
  }

  selected_area_.reset();

  update();
}

static void DrawTitle(QPainter& painter, const std::string& text) {
  const QFontMetrics font_metrics(painter.font());
  const QString qtext = QString::fromStdString(text);
  const QRect title_bounding_rect = font_metrics.boundingRect(qtext);
  const int text_start_to_center = (painter.device()->width() - title_bounding_rect.width()) / 2;
  painter.drawText(std::max(text_start_to_center, 0), title_bounding_rect.height(), qtext);
}

static void DrawSelection(QPainter& painter, int start_x, int end_x,
                          const QPoint& axes_intersection, int vertical_axis_length) {
  if (start_x == end_x) return;
  if (start_x > end_x) std::swap(start_x, end_x);

  const QPoint top_left = {start_x, axes_intersection.y() - vertical_axis_length};
  const QPoint bottom_right = {end_x, axes_intersection.y()};
  const QRect selection(top_left, bottom_right);
  painter.fillRect(selection, kSelectionColor);
}

void HistogramWidget::paintEvent(QPaintEvent* /*event*/) {
  QPainter painter(this);
  if (histogram_stack_.empty()) {
    DrawTitle(painter, "Select a function with Count>0 to plot a histogram of its runtime");
    return;
  }

  const orbit_statistics::Histogram& histogram = histogram_stack_.top();

  const int axis_width = painter.pen().width();

  QPoint axes_intersection(WidthMargin(), Height() - HeightMargin());

  const int vertical_axis_length = Height() - 2 * HeightMargin();
  const int horizontal_axis_length = Width() - 2 * WidthMargin();

  const uint64_t max_count =
      *std::max_element(std::begin(histogram.counts), std::end(histogram.counts));
  const double max_freq = static_cast<double>(max_count) / histogram.data_set_size;

  DrawHistogram(painter, axes_intersection, histogram, horizontal_axis_length, vertical_axis_length,
                max_freq, axis_width, MinValue());

  DrawHorizontalAxis(painter, axes_intersection, histogram, horizontal_axis_length, MinValue());
  DrawVerticalAxis(painter, axes_intersection, vertical_axis_length, max_freq);

  if (IsSelectionActive()) {
    DrawTitle(painter, absl::StrFormat("Selection over %u points", histogram.data_set_size));
  } else {
    DrawTitle(painter, function_data_->name);
  }

  if (selected_area_) {
    DrawSelection(painter, selected_area_->selection_start_pixel,
                  selected_area_->selection_current_pixel, axes_intersection, vertical_axis_length);
  }
}

void HistogramWidget::mousePressEvent(QMouseEvent* event) {
  if (histogram_stack_.empty()) return;

  const int pixel_x = event->x();
  selected_area_ = {pixel_x, pixel_x};

  update();
}

[[nodiscard]] static uint64_t LocationToValue(int pos_x, int width, int margin, uint64_t min_value,
                                              uint64_t max_value) {
  if (pos_x <= margin) return 0;
  if (pos_x > width - margin) return max_value + 1;

  const int location = pos_x - margin;
  const int histogram_width = width - 2 * margin;
  const uint64_t value_range = max_value - min_value;
  return min_value +
         static_cast<uint64_t>(static_cast<double>(location) / histogram_width * value_range);
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent* /* event*/) {
  if (histogram_stack_.empty()) return;

  if (selected_area_) {
    // if it wasn't a drag, but just a click, go one level of selections up
    if (selected_area_->selection_start_pixel == selected_area_->selection_current_pixel) {
      if (IsSelectionActive()) histogram_stack_.pop();
      selected_area_.reset();
      update();
      return;
    }

    uint64_t min = LocationToValue(selected_area_->selection_start_pixel, Width(), WidthMargin(),
                                   MinValue(), MaxValue());
    uint64_t max = LocationToValue(selected_area_->selection_current_pixel, Width(), WidthMargin(),
                                   MinValue(), MaxValue());
    if (min > max) {
      std::swap(min, max);
    }

    const auto data_begin = function_data_->data->begin();
    const auto data_end = function_data_->data->end();

    const auto min_it = std::lower_bound(data_begin, data_end, min);
    if (min_it != function_data_->data->end()) {
      const auto max_it = std::upper_bound(data_begin, data_end, max);
      const auto selection = absl::Span<const uint64_t>(&*min_it, std::distance(min_it, max_it));

      auto histogram = orbit_statistics::BuildHistogram(selection);
      if (histogram) {
        histogram_stack_.push(std::move(*histogram));
      }
    }
    selected_area_.reset();
  }

  update();
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* event) {
  if (!selected_area_) return;

  selected_area_->selection_current_pixel = event->x();

  update();
}

bool HistogramWidget::IsSelectionActive() const { return histogram_stack_.size() > 1; }

uint64_t HistogramWidget::MinValue() const {
  return IsSelectionActive() ? histogram_stack_.top().min : 0;
}

uint64_t HistogramWidget::MaxValue() const { return histogram_stack_.top().max; }

int HistogramWidget::Width() const { return size().width(); }

int HistogramWidget::Height() const { return size().height(); }

int HistogramWidget::HeightMargin() const { return RoundToClosestInt(Height() * kRelativeMargin); }

int HistogramWidget::WidthMargin() const { return RoundToClosestInt(Width() * kRelativeMargin); }