// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbithistogram.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <qevent.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qwidget.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>

#include "DisplayFormats/DisplayFormats.h"
#include "Statistics/Histogram.h"

constexpr double kRelativeMargin = 0.1;

constexpr uint32_t kTicksNum = 3;
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

static void DrawHorizontalAxis(QPainter& painter, const QPoint& zero,
                               const orbit_statistics::Histogram& histogram, int length) {
  DrawHorizontalLine(painter, zero, length);

  int tick_spacing_as_value = RoundToClosestInt(static_cast<double>(histogram.max) / kTicksNum);
  int tick_spacing_pixels = RoundToClosestInt(static_cast<double>(length) / kTicksNum);

  int current_tick_location = tick_spacing_pixels + zero.x();
  int current_tick_value = tick_spacing_as_value;

  QFontMetrics font_metrics(painter.font());

  for (uint32_t i = 1; i <= kTicksNum; ++i) {
    DrawVerticalLine(painter, {current_tick_location, zero.y()}, kTickLength);

    QString tick_label = QString::fromStdString(
        orbit_display_formats::GetDisplayTime(absl::Nanoseconds(current_tick_value)));
    QRect tick_label_bounding_rect = font_metrics.boundingRect(tick_label);
    painter.drawText(current_tick_location - tick_label_bounding_rect.width() / 2,
                     zero.y() + kTickLength + tick_label_bounding_rect.height(), tick_label);

    current_tick_location += tick_spacing_pixels;
    current_tick_value += tick_spacing_as_value;
  }
}

static void DrawVerticalAxis(QPainter& painter, const QPoint& zero, int length, double max_freq) {
  DrawVerticalLine(painter, zero, -length);

  double tick_spacing_as_value = max_freq / kTicksNum;
  double current_tick_value = tick_spacing_as_value;

  int tick_spacing_pixels = RoundToClosestInt(static_cast<double>(length) / kTicksNum);
  int current_tick_location = zero.y() - tick_spacing_pixels;

  QFontMetrics font_metrics(painter.font());

  for (uint32_t i = 1; i <= kTicksNum; ++i) {
    DrawHorizontalLine(painter, {zero.x(), current_tick_location}, -kTickLength);

    QString tick_label = QString::fromStdString(absl::StrFormat("%.2f", current_tick_value));

    QRect tick_label_bounding_rect = font_metrics.boundingRect(tick_label);
    painter.drawText(zero.x() - tick_label_bounding_rect.width() - kTickLength,
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

static void DrawHistogram(QPainter& painter, const QPoint& zero,
                          const orbit_statistics::Histogram& histogram, int horizontal_axis_length,
                          int vertical_axis_length, double max_freq) {
  for (size_t i = 0; i < histogram.counts.size(); ++i) {
    uint64_t bin_from = histogram.min + i * histogram.bin_width;
    uint64_t bin_to = bin_from + histogram.bin_width;

    double freq = GetFreq(histogram, i);
    if (freq > 0) {
      QPoint top_left(
          zero.x() + ValueToAxisLocation(bin_from, horizontal_axis_length, histogram.max),
          zero.y() - ValueToAxisLocation(freq, vertical_axis_length, max_freq));
      QPoint lower_right(
          zero.x() + ValueToAxisLocation(bin_to, horizontal_axis_length, histogram.max), zero.y());
      QRect bar(top_left, lower_right);
      painter.fillRect(bar, Qt::red);
    }
  }
}

void OrbitHistogram::SetHistogram(std::optional<orbit_statistics::Histogram> histogram,
                                  std::string function_name) {
  histogram_ = std::move(histogram);
  function_name_ = std::move(function_name);
}

void OrbitHistogram::paintEvent(QPaintEvent* /*event*/) {
  if (!histogram_) return;

  QPainter painter(this);
  int width = painter.device()->width();
  int height = painter.device()->height();

  QPoint zero(RoundToClosestInt(width * kRelativeMargin),
              RoundToClosestInt(height * (1 - kRelativeMargin)));

  const int vertical_axis_length = Height(painter) - 2 * HeightMargin(painter);
  const int horizontal_axis_length = Width(painter) - 2 * WidthMargin(painter);

  uint64_t max_count =
      *std::max_element(std::begin(histogram_->counts), std::end(histogram_->counts));
  double max_freq = static_cast<double>(max_count) / histogram_->data_set_size;

  DrawHistogram(painter, zero, histogram_.value(), horizontal_axis_length, vertical_axis_length,
                max_freq);

  DrawHorizontalAxis(painter, zero, histogram_.value(), horizontal_axis_length);
  DrawVerticalAxis(painter, zero, vertical_axis_length, max_freq);

  QString title = QString::fromStdString(function_name_.value());

  QFontMetrics font_metrics(painter.font());
  QRect title_bounding_rect = font_metrics.boundingRect(title);
  painter.drawText((width - title_bounding_rect.width()) / 2, title_bounding_rect.height(), title);
}