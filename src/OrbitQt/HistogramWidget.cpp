// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/HistogramWidget.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <stdlib.h>

#include <QColor>
#include <QFlags>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QStringLiteral>
#include <Qt>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ClientData/ScopeId.h"
#include "DisplayFormats/DisplayFormats.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Typedef.h"
#include "Statistics/Histogram.h"

using ::orbit_client_data::ScopeId;

namespace orbit_qt {

const QColor kBackgroundColor(QStringLiteral("#323232"));
constexpr size_t kBarColorsCount = 2;
const std::array<QColor, kBarColorsCount> kBarColors = {QColor(QStringLiteral("#2A82DA")),
                                                        QColor(QStringLiteral("#3198FF"))};
const QColor kHoveredBarColor(QStringLiteral("#99CCFF"));

constexpr int kHoverLabelPadding = 6;

namespace {
struct TickStep {
  double value{};
  int precision{};
};
}  // namespace

const std::vector<TickStep> kHorizontalTickSteps = {
    {0.001, 3}, {0.005, 3}, {0.01, 2}, {0.05, 2}, {0.1, 1}, {0.25, 2}, {0.5, 1},  {1, 0},    {5, 0},
    {10, 0},    {20, 0},    {50, 0},   {100, 0},  {500, 0}, {1000, 0}, {5000, 0}, {10000, 0}};

const std::vector<TickStep> kVerticalTickSteps = {{0.1, 1}, {0.5, 1}, {1, 0},
                                                  {5, 0},   {10, 0},  {25, 0}};
constexpr uint32_t kVerticalTickCount = 4;
constexpr uint32_t kHorizontalTickCount = 4;
constexpr int kVerticalAxisTickLength = 4;
constexpr int kHorizontalAxisTickLength = 8;
constexpr int kTickLabelGap = 3;

const QColor kAxisColor = Qt::white;
constexpr int kLineWidth = 2;

constexpr int kHintTopMargin = 10;
constexpr int kHintRightMargin = 50;
constexpr int kHintBottom = 40;
const QColor kHintFirstLineColor = Qt::white;
const QColor kHintSecondLineColor(QStringLiteral("#999999"));

constexpr int kVerticalLabelHeight = 15;
constexpr int kVerticalLabelWidth = 30;
const QColor kHoverLabelColor(QStringLiteral("#3f3f3f"));

constexpr int kTopMargin = 50;
constexpr int kBottomMargin = 40;
constexpr int kLeftMargin = 50;
constexpr int kRightMargin = 50;

const QString kDefaultTitle =
    QStringLiteral("Select a function with Count>0 to plot a histogram of its runtime");

const QColor kSelectionColor(QStringLiteral("#1B548C"));

[[nodiscard]] static int RoundToClosestInt(double x) { return static_cast<int>(std::round(x)); }

// if `length > 0`, the line will be plot to the right from `start` and to the left otherwise
static void DrawHorizontalLine(QPainter& painter, const QPoint& start, int length) {
  painter.drawLine(start, {start.x() + length, start.y()});
}

// if `length > 0`, the line will be plot downwards from `start` and upwards otherwise
static void DrawVerticalLine(QPainter& painter, const QPoint& start, int length) {
  painter.drawLine(start, {start.x(), start.y() + length});
}

[[nodiscard]] static uint32_t TickCount(double min, double max, double step) {
  double first = std::ceil(min / step) * step;
  if (first > max) return 0;
  return static_cast<uint32_t>(std::floor((max - first) / step)) + 1;
}

[[nodiscard]] static std::vector<double> MakeLabelValues(double min, double max, double step) {
  double current = std::ceil(min / step) * step;
  std::vector<double> result = {current};

  while (current <= max) {
    result.push_back(current);
    current += step;
  }

  return result;
}

// Providing exactly `optimal_tick_count` of ticks is impossible as we use a finite set of `steps`.
// Hence, we choose the step leading to the number of ticks closest to `optimal_tick_count`.
// If the available tick count is either 0 or 1, the step yielding zero ticks may be returned.
[[nodiscard]] static TickStep ChooseBestStep(double min, double max,
                                             absl::Span<const TickStep> steps,
                                             uint32_t optimal_tick_count) {
  std::optional<TickStep> best_step;
  int best_deviation = std::numeric_limits<int>::max();
  for (const TickStep& step : steps) {
    int tick_count = static_cast<int>(TickCount(min, max, step.value));
    if (best_step && tick_count < 2) continue;

    int deviation = std::abs(tick_count - static_cast<int>(optimal_tick_count));

    if (deviation < best_deviation) {
      best_step = step;
      best_deviation = deviation;
    }
  }
  return *best_step;
}

namespace {
struct Ticks {
  std::vector<QString> labels;
  std::vector<double> values;
  int precision{};
};
}  // namespace

[[nodiscard]] static Ticks MakeTicksFromValues(std::vector<double> values, int precision) {
  std::vector<QString> labels;
  std::transform(
      std::begin(values), std::end(values), std::back_inserter(labels),
      [precision](const double value) { return QString::number(value, 'f', precision); });
  return {labels, std::move(values), precision};
}

[[nodiscard]] static Ticks MakeTicks(double min, double max, absl::Span<const TickStep> steps,
                                     uint32_t optimal_tick_count) {
  TickStep step = ChooseBestStep(min, max, steps, optimal_tick_count);
  std::vector<double> values = MakeLabelValues(min, max, step.value);
  return MakeTicksFromValues(values, step.precision);
}

[[nodiscard]] static int ValueToAxisLocation(double value, int axis_length, double min_value,
                                             double max_value) {
  if (min_value == max_value) return 0;
  return RoundToClosestInt(((value - min_value) / (max_value - min_value)) * axis_length);
}

static void DrawHorizontalAxis(QPainter& painter, const QPoint& axes_intersection, int length,
                               const Ticks& ticks, int axis_length, double min_value,
                               double max_value) {
  DrawHorizontalLine(painter, axes_intersection, length);

  const QFontMetrics font_metrics(painter.font());

  for (size_t i = 0; i < ticks.values.size(); ++i) {
    const double tick_value = ticks.values[i];
    const int tick_location =
        ValueToAxisLocation(tick_value, axis_length, min_value, max_value) + axes_intersection.x();

    DrawVerticalLine(painter, {tick_location, axes_intersection.y()}, kHorizontalAxisTickLength);

    const QString& tick_label = ticks.labels[i];
    const QRect tick_label_bounding_rect = font_metrics.tightBoundingRect(tick_label);

    painter.drawText(tick_location - tick_label_bounding_rect.width() / 2,
                     axes_intersection.y() + kHorizontalAxisTickLength +
                         tick_label_bounding_rect.height() + kTickLabelGap + kLineWidth / 2,
                     tick_label);
  }
}

static void DrawVerticalAxis(QPainter& painter, const QPoint& axes_intersection, int length,
                             const Ticks& ticks, int axis_length, double max_value) {
  DrawVerticalLine(painter, axes_intersection, -length);

  const QFontMetrics font_metrics(painter.font());

  for (size_t i = 1; i < ticks.values.size(); ++i) {
    const double tick_value = ticks.values[i];
    const int tick_location =
        axes_intersection.y() - ValueToAxisLocation(tick_value, axis_length, 0.0, max_value);

    // We skip the ticks that do not fall into the horizontal axis range. Such ticks might appear
    // due to rounding errors.
    if (!(axes_intersection.y() - axis_length <= tick_location &&
          tick_location <= axes_intersection.y())) {
      continue;
    }

    DrawHorizontalLine(painter, {axes_intersection.x(), tick_location}, -kVerticalAxisTickLength);

    QString tick_label = ticks.labels[i];

    QRect tick_label_bounding_rect = font_metrics.tightBoundingRect(tick_label);
    painter.drawText(axes_intersection.x() - tick_label_bounding_rect.width() -
                         kVerticalAxisTickLength - kTickLabelGap - kLineWidth,
                     tick_location + (tick_label_bounding_rect.height()) / 2, tick_label);
  }
}

[[nodiscard]] static double GetFreq(const orbit_statistics::Histogram& histogram, size_t i) {
  return static_cast<double>(histogram.counts[i]) / static_cast<double>(histogram.data_set_size);
}

static void SetBoldFont(QPainter& painter) {
  QFont font = painter.font();
  font.setBold(true);
  painter.setFont(font);
}

static void DrawHoverLabel(QPainter& painter, const QRect& rect, const QString& text) {
  SetBoldFont(painter);
  painter.fillRect(rect, kHoverLabelColor);
  painter.drawText(rect, Qt::AlignCenter, text);
}

static void DrawVerticalHoverLabel(QPainter& painter, const QPoint& axes_intersection,
                                   const orbit_qt::BarData& bar_data, int decimals_count) {
  // We treat 100% frequency as a special case to render the value as "100", not as "100.0".
  // It doesn't fit into the widget otherwise.
  QString label_text = bar_data.frequency == 1.0
                           ? QStringLiteral("100")
                           : QString::number(bar_data.frequency * 100, 'f', decimals_count);

  QRect label_rect(QPoint(0, 0), QPoint(kVerticalLabelWidth, kVerticalLabelHeight));

  label_rect.moveTo(axes_intersection.x() - label_rect.width() - kLineWidth / 2 -
                        kVerticalAxisTickLength - kTickLabelGap,
                    bar_data.top_y_pos - label_rect.height() / 2);
  DrawHoverLabel(painter, label_rect, label_text);
}

static void DrawHistogram(QPainter& painter, const QPoint& axes_intersection,
                          const orbit_statistics::Histogram& histogram, int horizontal_axis_length,
                          int vertical_axis_length, double max_freq, uint64_t min_value,
                          const std::optional<int>& histogram_hover_x,
                          int vertical_label_decimal_count) {
  int color_index = 0;
  std::optional<orbit_qt::BarData> hovered_bar_data;
  const int first_bar_offset_from_axes_intersection =
      ValueToAxisLocation(histogram.min, horizontal_axis_length, min_value, histogram.max);
  int left_x = axes_intersection.x() + kLineWidth / 2 + first_bar_offset_from_axes_intersection;
  std::vector<int> widths = orbit_qt::GenerateHistogramBinWidths(
      histogram.counts.size(),
      horizontal_axis_length - first_bar_offset_from_axes_intersection + 1);

  // If the number of bins exceeds the width of histogram in pixels, `widths[i]` might be zero.
  // In such case we plot the bar on top of the previous one
  // Because of that we keep track of hovered_bar_data (multiple bars may be hovered at once).
  // As we render the tallest bar, the hover label shows the highest frequency
  for (size_t i = 0; i < histogram.counts.size(); ++i) {
    double freq = GetFreq(histogram, i);
    if (freq > 0) {
      const int top_y = axes_intersection.y() - kLineWidth -
                        ValueToAxisLocation(freq, vertical_axis_length, 0, max_freq);
      const int right_x = left_x + std::max(widths[i] - 1, 0);
      const QPoint top_left(left_x, top_y);
      const QPoint bottom_right(right_x, axes_intersection.y() - kLineWidth);
      const QRect bar(top_left, bottom_right);

      const bool is_bar_hovered = histogram_hover_x.has_value() && left_x <= *histogram_hover_x &&
                                  *histogram_hover_x <= right_x;

      const QColor& bar_color =
          is_bar_hovered ? kHoveredBarColor : kBarColors[color_index % kBarColors.size()];
      painter.fillRect(bar, bar_color);

      const bool current_bar_is_taller = !hovered_bar_data || hovered_bar_data->frequency < freq;

      if (is_bar_hovered && current_bar_is_taller) {
        hovered_bar_data = {freq, top_y};
      }
    }

    if (widths[i] > 0) color_index++;
    left_x += widths[i];
  }

  if (hovered_bar_data) {
    DrawVerticalHoverLabel(painter, axes_intersection, *hovered_bar_data,
                           vertical_label_decimal_count);
  }
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

[[nodiscard]] static uint64_t LocationToValue(int pos_x, int width, uint64_t min_value,
                                              uint64_t max_value) {
  if (pos_x <= kLeftMargin) return min_value;
  if (pos_x > width - kRightMargin) return max_value;

  const int location = pos_x - kLeftMargin;
  const int histogram_width = width - kLeftMargin - kRightMargin;
  const uint64_t value_range = max_value - min_value;
  return min_value +
         static_cast<uint64_t>(static_cast<double>(location) / histogram_width * value_range);
}

static void DrawHorizontalHoverLabel(QPainter& painter, const QPoint& axes_intersection,
                                     std::optional<int> histogram_hover_x, int width,
                                     uint64_t min_value, uint64_t max_value,
                                     orbit_display_formats::TimeUnit time_unit,
                                     int decimals_count) {
  if (!histogram_hover_x) return;

  const uint64_t value_pos = LocationToValue(*histogram_hover_x, width, min_value, max_value);
  const QString label_text = QString::number(
      orbit_display_formats::ToDoubleInGivenTimeUnits(absl::Nanoseconds(value_pos), time_unit), 'f',
      decimals_count);

  const QFontMetrics font_metrics(painter.font());
  const QRect bounding_rect = font_metrics.tightBoundingRect(label_text);

  QRect label_rect(bounding_rect.topLeft(),
                   bounding_rect.bottomRight() + QPoint(kHoverLabelPadding, kHoverLabelPadding));
  label_rect.moveTo(*histogram_hover_x - bounding_rect.width() / 2,
                    axes_intersection.y() + kHorizontalAxisTickLength + kTickLabelGap);

  DrawHoverLabel(painter, label_rect, label_text);
}

static void DrawOneLineOfHint(QPainter& painter, const QString& message, const QPoint& bottom_right,
                              const QColor& color) {
  painter.setPen(color);
  const QRect rect(QPoint(0, 0), bottom_right);
  painter.drawText(rect, Qt::AlignRight | Qt::AlignBottom, message);
}

static void DrawHint(QPainter& painter, int width, orbit_display_formats::TimeUnit time_unit) {
  const QString first_line =
      QString::fromStdString(absl::StrFormat("Distribution (%%) / Execution time (%s)",
                                             orbit_display_formats::GetDisplayTimeUnit(time_unit)));
  const QString second_line =
      QStringLiteral("Drag over a selection to zoom in or click to zoom out");

  const QFontMetrics font_metrics(painter.font());

  const QRect first_bounding_rect = font_metrics.tightBoundingRect(first_line);
  DrawOneLineOfHint(painter, first_line,
                    QPoint(width - kHintRightMargin, kHintTopMargin + first_bounding_rect.height()),
                    kHintFirstLineColor);
  DrawOneLineOfHint(painter, second_line, QPoint(width - kHintRightMargin, kHintBottom),
                    kHintSecondLineColor);
}

constexpr uint32_t kSeed = 31;

[[nodiscard]] std::vector<int> GenerateHistogramBinWidths(size_t number_of_bins,
                                                          int histogram_width) {
  std::mt19937 gen32(kSeed);

  const int narrower_width = histogram_width / number_of_bins;
  const int wider_width = narrower_width + 1;

  const int number_of_wider_bins = histogram_width % number_of_bins;
  const int number_of_narrower_bins = number_of_bins - number_of_wider_bins;

  std::vector<int> result(number_of_narrower_bins, narrower_width);
  const std::vector<int> wider_widths(number_of_wider_bins, wider_width);
  result.insert(std::end(result), std::begin(wider_widths), std::end(wider_widths));

  // shuffle the result for the histogram to look more natural
  std::shuffle(std::begin(result), std::end(result), std::default_random_engine{});
  return result;
}

void HistogramWidget::UpdateData(const std::vector<uint64_t>* data, std::string scope_name,
                                 std::optional<ScopeId> scope_id) {
  ORBIT_SCOPE_FUNCTION;
  if (scope_data_.has_value() && scope_data_->id == scope_id) return;

  histogram_stack_ = {};
  ranges_stack_ = {};
  EmitSignalSelectionRangeChange();

  if (scope_id.has_value()) {
    scope_data_.emplace(data, std::move(scope_name), scope_id.value());
  } else {
    scope_data_ = std::nullopt;
  }

  if (scope_data_.has_value() && data != nullptr) {
    std::optional<orbit_statistics::Histogram> histogram =
        orbit_statistics::BuildHistogram(*scope_data_.value().data);
    if (histogram) {
      histogram_stack_.push(std::move(*histogram));
    }
  }

  selected_area_.reset();

  EmitSignalTitleChange();
  update();
}

[[nodiscard]] static double NanosecondsToDoubleInGivenUnits(
    uint64_t nanos, orbit_display_formats::TimeUnit time_unit) {
  return orbit_display_formats::ToDoubleInGivenTimeUnits(absl::Nanoseconds(nanos), time_unit);
}

void HistogramWidget::paintEvent(QPaintEvent* /*event*/) {
  if (histogram_stack_.empty()) {
    return;
  }

  QPainter painter(this);

  painter.fillRect(0, 0, Width(), Height(), kBackgroundColor);

  const orbit_statistics::Histogram& histogram = histogram_stack_.top();

  const QPoint axes_intersection(kLeftMargin, Height() - kBottomMargin);

  const int vertical_axis_length = Height() - kTopMargin - kBottomMargin;
  const int horizontal_axis_length = Width() - kLeftMargin - kRightMargin;

  const uint64_t max_count =
      *std::max_element(std::begin(histogram.counts), std::end(histogram.counts));
  const double max_freq = static_cast<double>(max_count) / histogram.data_set_size;

  if (selected_area_) {
    DrawSelection(painter, selected_area_->selection_start_pixel,
                  selected_area_->selection_current_pixel, axes_intersection, vertical_axis_length);
  }
  const auto tick_spacing_as_value = (histogram.max - MinValue()) / (kHorizontalTickCount - 1);
  orbit_display_formats::TimeUnit time_unit = orbit_display_formats::ChooseUnitForDisplayTime(
      absl::Nanoseconds(MinValue() + tick_spacing_as_value));

  const double min_value_in_units = NanosecondsToDoubleInGivenUnits(MinValue(), time_unit);
  const double max_value_in_units = NanosecondsToDoubleInGivenUnits(MaxValue(), time_unit);
  const Ticks horizontal_ticks =
      MakeTicks(min_value_in_units, max_value_in_units, kHorizontalTickSteps, kHorizontalTickCount);

  const Ticks vertical_ticks =
      MakeTicks(0.0, max_freq * 100.0, kVerticalTickSteps, kVerticalTickCount);

  DrawHint(painter, Width(), time_unit);

  painter.setPen(QPen(kAxisColor, kLineWidth));

  DrawHorizontalAxis(painter, axes_intersection, horizontal_axis_length, horizontal_ticks,
                     horizontal_axis_length, min_value_in_units, max_value_in_units);
  DrawVerticalAxis(painter, axes_intersection, vertical_axis_length, vertical_ticks,
                   vertical_axis_length, max_freq * 100.0);
  painter.setPen(QPen(Qt::white, 1));

  DrawHistogram(painter, axes_intersection, histogram, horizontal_axis_length, vertical_axis_length,
                max_freq, MinValue(), histogram_hover_x_, 1);

  DrawHorizontalHoverLabel(painter, axes_intersection, histogram_hover_x_, Width(), MinValue(),
                           MaxValue(), time_unit, horizontal_ticks.precision + 1);
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent* /* event*/) {
  if (histogram_stack_.empty()) return;

  ORBIT_SCOPE("Histogram zooming in");
  if (selected_area_) {
    // if it wasn't a drag, but just a click, go one level of selections up
    if (selected_area_->selection_start_pixel == selected_area_->selection_current_pixel) {
      if (IsSelectionActive()) {
        histogram_stack_.pop();
        ranges_stack_.pop();
      }
      selected_area_.reset();
      UpdateAndNotify();
      return;
    }

    uint64_t min =
        LocationToValue(selected_area_->selection_start_pixel, Width(), MinValue(), MaxValue());
    uint64_t max =
        LocationToValue(selected_area_->selection_current_pixel, Width(), MinValue(), MaxValue());
    if (min > max) {
      std::swap(min, max);
    }

    const auto data_begin = scope_data_->data->begin();
    const auto data_end = scope_data_->data->end();

    const auto min_it = std::lower_bound(data_begin, data_end, min);
    if (min_it != scope_data_->data->end()) {
      const auto max_it = std::upper_bound(data_begin, data_end, max);
      const auto selection = absl::Span<const uint64_t>(&*min_it, std::distance(min_it, max_it));

      if (selection.front() == MinValue() && selection.back() == MaxValue()) {
        selected_area_.reset();
        UpdateAndNotify();
        return;
      }

      auto histogram = orbit_statistics::BuildHistogram(selection);
      if (histogram) {
        histogram_stack_.push(std::move(*histogram));
        ranges_stack_.push({min, max});
      }
    }
    selected_area_.reset();
  }

  UpdateAndNotify();
}

void HistogramWidget::mousePressEvent(QMouseEvent* event) {
  if (histogram_stack_.empty()) return;

  const int pixel_x = event->x();
  selected_area_ = {pixel_x, pixel_x};

  update();
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* event) {
  if (IsOverHistogram(event->pos())) {
    histogram_hover_x_ = event->x();
  } else {
    histogram_hover_x_ = std::nullopt;
  }

  if (selected_area_) selected_area_->selection_current_pixel = event->x();

  update();
}

bool HistogramWidget::IsSelectionActive() const { return histogram_stack_.size() > 1; }

uint64_t HistogramWidget::MinValue() const {
  return IsSelectionActive() ? histogram_stack_.top().min : 0;
}

uint64_t HistogramWidget::MaxValue() const { return histogram_stack_.top().max; }

int HistogramWidget::Width() const { return size().width(); }

int HistogramWidget::Height() const { return size().height(); }

[[nodiscard]] std::optional<orbit_statistics::HistogramSelectionRange>
HistogramWidget::GetSelectionRange() const {
  if (ranges_stack_.empty()) {
    return std::nullopt;
  }
  return ranges_stack_.top();
}

void HistogramWidget::EmitSignalSelectionRangeChange() const {
  emit SignalSelectionRangeChange(GetSelectionRange());
}

void HistogramWidget::EmitSignalTitleChange() const { emit SignalTitleChange(GetTitle()); }

void HistogramWidget::UpdateAndNotify() {
  EmitSignalSelectionRangeChange();
  EmitSignalTitleChange();
  update();
}

constexpr size_t kMaxScopeNameLengthForTitle = 80;

[[nodiscard]] QString HistogramWidget::GetTitle() const {
  if (!scope_data_.has_value() || histogram_stack_.empty()) {
    return kDefaultTitle;
  }
  std::string scope_name = scope_data_->name;
  if (scope_name.size() > kMaxScopeNameLengthForTitle) {
    scope_name = absl::StrCat(scope_name.substr(0, kMaxScopeNameLengthForTitle), "...");
  }

  scope_name = absl::StrReplaceAll(scope_name, {{"&", "&amp;"}, {"<", "&lt;"}, {">", "&gt;"}});

  std::string title =
      absl::StrFormat("<b>%s</b> (%d of %d hits)", scope_name, histogram_stack_.top().data_set_size,
                      scope_data_->data->size());

  return QString::fromStdString(title);
}

[[nodiscard]] bool HistogramWidget::IsOverHistogram(const QPoint& pos) const {
  return kLeftMargin <= pos.x() && pos.x() <= Width() - kRightMargin && kTopMargin <= pos.y() &&
         pos.y() <= Height() - kBottomMargin;
}

}  // namespace orbit_qt