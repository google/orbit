// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "HistogramWidget.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <qnamespace.h>
#include <qpoint.h>

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

#include "DisplayFormats/DisplayFormats.h"
#include "Statistics/Histogram.h"

constexpr double kRelativeMargin = 0.1;

constexpr uint32_t kVerticalTickCount = 3;
constexpr uint32_t kHorizontalTickCount = 3;
constexpr int kTickLength = 5;

constexpr QColor kSelectionColor = QColor(128, 128, 255, 128);

[[nodiscard]] static int RoundToClosestInt(double x) { return static_cast<int>(std::round(x)); }

[[nodiscard]] static int Width(const QPainter& painter) { return painter.device()->width(); }

[[nodiscard]] static int WidthMargin(const QPainter& painter) {
  return RoundToClosestInt(Width(painter) * kRelativeMargin);
}

[[nodiscard]] static int Height(const QPainter& painter) { return painter.device()->height(); }

[[nodiscard]] static int HeightMargin(const QPainter& painter) {
  return RoundToClosestInt(Height(painter) * kRelativeMargin);
}

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

  const int tick_spacing_as_value =
      RoundToClosestInt(static_cast<double>(histogram.max - min_value) / kHorizontalTickCount);
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

void HistogramWidget::UpdateData(std::vector<uint64_t> data, std::string function_name) {
  histogram_stack_ = {};

  data_ = std::move(data);
  std::sort(data_->begin(), data_->end());

  function_name_ = std::move(function_name);

  std::optional<orbit_statistics::Histogram> histogram =
      orbit_statistics::BuildHistogram(data_.value());
  if (histogram) {
    histogram_stack_.push(std::move(*histogram));
  }

  selection_start_pixel.reset();
  selection_current_pixel.reset();

  update();
}

static void DrawTitle(QPainter& painter, const std::string& text) {
  const QFontMetrics font_metrics(painter.font());
  const QString qtext = QString::fromStdString(text);
  const QRect title_bounding_rect = font_metrics.boundingRect(qtext);
  painter.drawText((painter.device()->width() - title_bounding_rect.width()) / 2,
                   title_bounding_rect.height(), qtext);
}

static void DrawSelection(QPainter& painter, int start_x, int end_x,
                          const QPoint& axes_intersection, int vertical_axis_length) {
  if (start_x == end_x) return;
  if (start_x > end_x) std::swap(start_x, end_x);

  QPoint top_left = {start_x, axes_intersection.y() - vertical_axis_length};
  QPoint bottom_right = {end_x, axes_intersection.y()};
  QRect selection(top_left, bottom_right);
  painter.fillRect(selection, kSelectionColor);
}

void HistogramWidget::paintEvent(QPaintEvent* /*event*/) {
  QPainter painter(this);
  if (histogram_stack_.empty()) {
    DrawTitle(painter, "Select a function with Count>0 to plot a histogram of its runtime");
    return;
  }

  const bool is_selection_active = histogram_stack_.size() > 1;
  const orbit_statistics::Histogram& histogram = histogram_stack_.top();
  const uint64_t min_value = is_selection_active ? histogram.min : 0;

  const int axis_width = painter.pen().width();

  const int width = size().width();
  const int height = size().height();

  QPoint axes_intersection(RoundToClosestInt(width * kRelativeMargin),
                           RoundToClosestInt(height * (1 - kRelativeMargin)));

  const int vertical_axis_length = Height(painter) - 2 * HeightMargin(painter);
  const int horizontal_axis_length = Width(painter) - 2 * WidthMargin(painter);

  const uint64_t max_count =
      *std::max_element(std::begin(histogram.counts), std::end(histogram.counts));
  const double max_freq = static_cast<double>(max_count) / histogram.data_set_size;

  DrawHistogram(painter, axes_intersection, histogram, horizontal_axis_length, vertical_axis_length,
                max_freq, axis_width, min_value);

  DrawHorizontalAxis(painter, axes_intersection, histogram, horizontal_axis_length, min_value);
  DrawVerticalAxis(painter, axes_intersection, vertical_axis_length, max_freq);

  if (is_selection_active) {
    DrawTitle(painter, absl::StrFormat("Selection over %u points", histogram.data_set_size));
  } else {
    DrawTitle(painter, function_name_.value());
  }

  if (selection_start_pixel && selection_current_pixel) {
    DrawSelection(painter, *selection_start_pixel, *selection_current_pixel, axes_intersection,
                  vertical_axis_length);
  }
}

void HistogramWidget::mousePressEvent(QMouseEvent* event) {
  if (histogram_stack_.empty()) return;

  int pixel_x = event->x();
  selection_start_pixel = pixel_x;
  selection_current_pixel = pixel_x;

  update();
}

[[nodiscard]] static uint64_t LocationToValue(int pos_x, int width, uint64_t min_value,
                                              uint64_t max_value) {
  int margin = RoundToClosestInt(width * kRelativeMargin);
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

  const int width = size().width();
  const bool is_selection_active = histogram_stack_.size() > 1;
  if (selection_start_pixel && selection_current_pixel) {
    // if it wasn't a drag, but just a click, resent selection_ to return to the full histogram view
    if (*selection_start_pixel == *selection_current_pixel) {
      if (is_selection_active) histogram_stack_.pop();
      selection_start_pixel.reset();
      selection_current_pixel.reset();
      update();
      return;
    }

    uint64_t min_value = is_selection_active ? histogram_stack_.top().min : 0;
    uint64_t min =
        LocationToValue(*selection_start_pixel, width, min_value, histogram_stack_.top().max);
    uint64_t max =
        LocationToValue(*selection_current_pixel, width, min_value, histogram_stack_.top().max);
    std::cout << std::endl
              << min << " " << max << " " << min_value << " " << histogram_stack_.top().max
              << std::endl;
    if (min > max) {
      std::swap(min, max);
    }

    auto min_it = std::lower_bound(data_->begin(), data_->end(), min);
    auto max_it = std::upper_bound(data_->begin(), data_->end(), max);
    auto selection = absl::Span<const uint64_t>(&*min_it, std::distance(min_it, max_it));

    auto histogram = orbit_statistics::BuildHistogram(selection);
    if (histogram) {
      histogram_stack_.push(std::move(*histogram));
    }

    selection_start_pixel.reset();
    selection_current_pixel.reset();
  }

  update();
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* event) {
  if (!selection_start_pixel) return;

  selection_current_pixel = event->x();

  update();
}