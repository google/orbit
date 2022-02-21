// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "HistogramWidget.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <qnamespace.h>
#include <qpoint.h>

#include <QEvent>
#include <QPainter>
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
                               const orbit_statistics::Histogram& histogram, int length) {
  DrawHorizontalLine(painter, axes_intersection, length);

  const int tick_spacing_as_value =
      RoundToClosestInt(static_cast<double>(histogram.max) / kHorizontalTickCount);
  const int tick_spacing_pixels =
      RoundToClosestInt(static_cast<double>(length) / kHorizontalTickCount);

  int current_tick_location = tick_spacing_pixels + axes_intersection.x();
  int current_tick_value = tick_spacing_as_value;

  const QFontMetrics font_metrics(painter.font());

  for (uint32_t i = 1; i <= kHorizontalTickCount; ++i) {
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

[[nodiscard]] static int ValueToAxisLocation(double value, int axis_length, double max_value) {
  return RoundToClosestInt((value / max_value) * axis_length);
}

[[nodiscard]] static double GetFreq(const orbit_statistics::Histogram& histogram, size_t i) {
  return static_cast<double>(histogram.counts[i]) / static_cast<double>(histogram.data_set_size);
}

static void DrawHistogram(QPainter& painter, const QPoint& axes_intersection,
                          const orbit_statistics::Histogram& histogram, int horizontal_axis_length,
                          int vertical_axis_length, double max_freq, int vertical_shift) {
  for (size_t i = 0; i < histogram.counts.size(); ++i) {
    const uint64_t bin_from = histogram.min + i * histogram.bin_width;
    const uint64_t bin_to = bin_from + histogram.bin_width;

    double freq = GetFreq(histogram, i);
    if (freq > 0) {
      const QPoint top_left(
          axes_intersection.x() +
              ValueToAxisLocation(bin_from, horizontal_axis_length, histogram.max),
          axes_intersection.y() - vertical_shift -
              ValueToAxisLocation(freq, vertical_axis_length, max_freq));
      const QPoint lower_right(
          axes_intersection.x() +
              ValueToAxisLocation(bin_to, horizontal_axis_length, histogram.max),
          axes_intersection.y() - vertical_shift);
      const QRect bar(top_left, lower_right);
      painter.fillRect(bar, Qt::cyan);
    }
  }
}

void HistogramWidget::UpdateHistogram(std::optional<orbit_statistics::Histogram> histogram,
                                      std::string function_name) {
  histogram_ = std::move(histogram);
  function_name_ = std::move(function_name);
  update();
}

static void DrawTitle(const std::string& text, QPainter& painter) {
  const QFontMetrics font_metrics(painter.font());
  const QString qtext = QString::fromStdString(text);
  const QRect title_bounding_rect = font_metrics.boundingRect(qtext);
  painter.drawText((painter.device()->width() - title_bounding_rect.width()) / 2,
                   title_bounding_rect.height(), qtext);
}

void HistogramWidget::paintEvent(QPaintEvent* /*event*/) {
  QPainter painter(this);
  if (!histogram_) {
    DrawTitle("Select a function with Count>0 to plot a histogram of its runtime", painter);
    return;
  }

  const int axis_width = painter.pen().width();

  const int width = painter.device()->width();
  const int height = painter.device()->height();

  QPoint axes_intersection(RoundToClosestInt(width * kRelativeMargin),
                           RoundToClosestInt(height * (1 - kRelativeMargin)));

  const int vertical_axis_length = Height(painter) - 2 * HeightMargin(painter);
  const int horizontal_axis_length = Width(painter) - 2 * WidthMargin(painter);

  const uint64_t max_count =
      *std::max_element(std::begin(histogram_->counts), std::end(histogram_->counts));
  const double max_freq = static_cast<double>(max_count) / histogram_->data_set_size;

  DrawHistogram(painter, axes_intersection, histogram_.value(), horizontal_axis_length,
                vertical_axis_length, max_freq, axis_width);

  DrawHorizontalAxis(painter, axes_intersection, histogram_.value(), horizontal_axis_length);
  DrawVerticalAxis(painter, axes_intersection, vertical_axis_length, max_freq);

  DrawTitle(function_name_.value(), painter);
}