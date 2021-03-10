// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeViewer/Viewer.h"

#include <QDebug>
#include <QFontDatabase>
#include <QHeaderView>
#include <QMargins>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QPlainTextEdit>
#include <QRect>
#include <QResizeEvent>
#include <QScrollBar>
#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QWheelEvent>
#include <cmath>

namespace orbit_code_viewer {

static const QColor kLineNumberBackgroundColor{50, 50, 50};
static const QColor kLineNumberForegroundColor{189, 189, 189};
static const QColor kTextEditBackgroundColor{30, 30, 30};
static const QColor kTextEditForegroundColor{189, 189, 189};
static const QColor kTitleBackgroundColor{30, 65, 89};
static const QColor kHeatmapColor{Qt::red};

static int StringWidthInPixels(const QFontMetrics& font_metrics, QString string) {
  return font_metrics.horizontalAdvance(string);
}

int DetermineLineNumberWidthInPixels(const QFontMetrics& font_metrics, int max_line_number) {
  return StringWidthInPixels(font_metrics, QString::number(max_line_number));
}

Viewer::Viewer(QWidget* parent)
    : QPlainTextEdit(parent),
      top_bar_widget_{this},
      left_sidebar_widget_{this},
      right_sidebar_widget_{this} {
  UpdateBarsSize();
  QObject::connect(this, &QPlainTextEdit::blockCountChanged, this, &Viewer::UpdateBarsSize);

  QObject::connect(&top_bar_widget_, &PlaceHolderWidget::PaintEventTriggered, this,
                   &Viewer::DrawTopWidget);
  QObject::connect(&top_bar_widget_, &PlaceHolderWidget::WheelEventTriggered, this,
                   &Viewer::wheelEvent);

  QObject::connect(&left_sidebar_widget_, &PlaceHolderWidget::PaintEventTriggered, this,
                   &Viewer::DrawLineNumbers);
  QObject::connect(&left_sidebar_widget_, &PlaceHolderWidget::WheelEventTriggered, this,
                   &Viewer::wheelEvent);

  QObject::connect(&right_sidebar_widget_, &PlaceHolderWidget::PaintEventTriggered, this,
                   &Viewer::DrawSampleCounters);
  QObject::connect(&right_sidebar_widget_, &PlaceHolderWidget::WheelEventTriggered, this,
                   &Viewer::wheelEvent);

  const auto update_viewport_area = [&](const QRect& rect, int dy) {
    bool update_caused_by_scroll = (dy != 0);
    if (update_caused_by_scroll) {
      left_sidebar_widget_.scroll(0, dy);
      right_sidebar_widget_.scroll(0, dy);
    } else {
      QRect titles_bar = rect;
      QRect line_number_rect = rect;
      QRect samples_info_rect = rect;

      // We keep the (vertical) y-coordinates and adjust the horizontal x-coordinates for the each
      // bar's area
      titles_bar.setLeft(0);
      titles_bar.setWidth(top_bar_widget_.sizeHint().width());
      top_bar_widget_.update(line_number_rect);

      line_number_rect.setLeft(0);
      line_number_rect.setWidth(left_sidebar_widget_.sizeHint().width());
      left_sidebar_widget_.update(line_number_rect);

      int total_width_without_scroll_bar = width() - verticalScrollBar()->width();
      int right_sidebar_width = right_sidebar_widget_.sizeHint().width();
      samples_info_rect.setLeft(total_width_without_scroll_bar - right_sidebar_width);
      samples_info_rect.setWidth(right_sidebar_width);
      right_sidebar_widget_.update(line_number_rect);
    }
  };
  QObject::connect(this, &QPlainTextEdit::updateRequest, this, update_viewport_area);

  setFont(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont));

  constexpr int kTabStopInWhitespaces = 4;
  setTabStopDistance(fontMetrics().horizontalAdvance(' ') * kTabStopInWhitespaces);

  setWordWrapMode(QTextOption::NoWrap);

  QPalette current_palette = palette();
  current_palette.setColor(QPalette::Base, kTextEditBackgroundColor);
  current_palette.setColor(QPalette::Text, kTextEditForegroundColor);
  setPalette(current_palette);
}

void Viewer::resizeEvent(QResizeEvent* ev) {
  QPlainTextEdit::resizeEvent(ev);

  UpdateBarsPosition();
}

void Viewer::wheelEvent(QWheelEvent* ev) {
  QPlainTextEdit::wheelEvent(ev);

  UpdateBarsSize();
  UpdateBarsPosition();
}

void Viewer::DrawTopWidget(QPaintEvent* event) {
  QPainter painter{&top_bar_widget_};
  painter.setFont(font());
  painter.fillRect(event->rect(), kTitleBackgroundColor);

  if (line_numbers_enabled_) {
    const int left = (left_margin_ + heatmap_bar_width_).ToPixels(fontMetrics());
    const int width = DetermineLineNumberWidthInPixels(fontMetrics(), blockCount());
    const QRect bounding_box{left, 0, width, fontMetrics().height()};

    painter.setPen(kLineNumberForegroundColor);
    painter.drawText(bounding_box, Qt::AlignCenter, QString("#"));
  }
  if (sample_counters_enabled_) {
    const int left = top_bar_widget_.width() - right_sidebar_widget_.width();

    int current = left;
    current += left_margin_.ToPixels(fontMetrics());
    {
      const QRect bounding_box{current, 0, WidthSampleCounterColumn(), fontMetrics().height()};
      painter.setPen(kLineNumberForegroundColor);
      painter.drawText(bounding_box, Qt::AlignCenter, QString("Samples"));
      current += WidthSampleCounterColumn() + WidthMarginBetweenColumns();
    }
    {
      const QRect bounding_box{current, 0, WidthPercentageColumn(), fontMetrics().height()};
      painter.setPen(kLineNumberForegroundColor);
      painter.drawText(bounding_box, Qt::AlignCenter, QString("Function"));
      current += WidthPercentageColumn() + WidthMarginBetweenColumns();
    }
    {
      const QRect bounding_box{current, 0, WidthPercentageColumn(), fontMetrics().height()};
      painter.setPen(kLineNumberForegroundColor);
      painter.drawText(bounding_box, Qt::AlignCenter, QString("Total"));
      current += WidthPercentageColumn() + WidthMarginBetweenColumns();
    }
  }
}

void Viewer::DrawLineNumbers(QPaintEvent* event) {
  QPainter painter{&left_sidebar_widget_};
  painter.setFont(font());
  painter.fillRect(event->rect(), kLineNumberBackgroundColor);

  const auto top_of = [&](const QTextBlock& block) {
    return static_cast<int>(
        std::round(blockBoundingGeometry(block).translated(contentOffset()).top()));
  };
  const auto bottom_of = [&](const QTextBlock& block) {
    return top_of(block) + static_cast<int>(std::round(blockBoundingRect(block).height()));
  };

  for (auto block = firstVisibleBlock(); block.isValid() && top_of(block) <= event->rect().bottom();
       block = block.next()) {
    if (!block.isVisible() || bottom_of(block) < event->rect().top()) continue;

    if (heatmap_bar_width_.Value() > 0.0 && code_report_ != nullptr) {
      const QRect heatmap_rect{0, top_of(block), heatmap_bar_width_.ToPixels(fontMetrics()),
                               fontMetrics().height()};

      const uint32_t num_samples_in_line =
          code_report_->GetNumSamplesAtLine(block.blockNumber() + 1).value_or(0);
      const uint32_t num_samples_in_function = code_report_->GetNumSamplesInFunction();
      const float intensity = std::clamp(
          static_cast<float>(num_samples_in_line) / static_cast<float>(num_samples_in_function),
          0.0f, 1.0f);
      const auto scaled_intensity = static_cast<int>(std::sqrt(intensity) * 255);
      QColor color = kHeatmapColor;
      color.setAlpha(scaled_intensity);

      painter.fillRect(heatmap_rect, color);
    }

    if (line_numbers_enabled_) {
      const int left = (left_margin_ + heatmap_bar_width_).ToPixels(fontMetrics());
      const int width = DetermineLineNumberWidthInPixels(fontMetrics(), blockCount());
      const QRect bounding_box{left, top_of(block), width, fontMetrics().height()};

      painter.setPen(kLineNumberForegroundColor);
      painter.drawText(bounding_box, Qt::AlignRight, QString::number(block.blockNumber() + 1));
    }
  }
}

// For example, from a value = 0.5, we want to print "50.00 %"
static QString FractionToPercentageString(int a, int b) {
  double ratio = (!b ? 0. : static_cast<double>(a) / b);
  double percentage_ratio = std::clamp(ratio * 100., 0., 100.);
  return QString{"%1 %"}.arg(percentage_ratio, 0, 'f', 2);
}

void Viewer::DrawSampleCounters(QPaintEvent* event) {
  if (!sample_counters_enabled_) return;
  QPainter painter{&right_sidebar_widget_};
  painter.setFont(font());
  painter.fillRect(event->rect(), kLineNumberBackgroundColor);

  const auto top_of = [&](const QTextBlock& block) {
    return static_cast<int>(
        std::round(blockBoundingGeometry(block).translated(contentOffset()).top()));
  };
  const auto bottom_of = [&](const QTextBlock& block) {
    return top_of(block) + static_cast<int>(std::round(blockBoundingRect(block).height()));
  };

  const int left = left_margin_.ToPixels(fontMetrics());
  for (auto block = firstVisibleBlock(); block.isValid() && top_of(block) <= event->rect().bottom();
       block = block.next()) {
    if (!block.isVisible() || bottom_of(block) < event->rect().top()) continue;

    auto maybe_num_samples_in_lines = code_report_->GetNumSamplesAtLine(block.blockNumber() + 1);
    if (!maybe_num_samples_in_lines.has_value()) continue;

    const uint32_t num_samples_in_line = maybe_num_samples_in_lines.value();
    // Draw sample counter
    int current = left;
    {
      const QRect bounding_box{current, top_of(block), WidthSampleCounterColumn(),
                               fontMetrics().height()};
      painter.setPen(kLineNumberForegroundColor);
      painter.drawText(bounding_box, Qt::AlignRight, QString::number(num_samples_in_line));
      current += WidthSampleCounterColumn() + WidthMarginBetweenColumns();
    }

    // Draw function percentage
    {
      const QRect bounding_box{current, top_of(block), WidthPercentageColumn(),
                               fontMetrics().height()};
      painter.setPen(kLineNumberForegroundColor);

      QString function_percetange_string =
          FractionToPercentageString(num_samples_in_line, code_report_->GetNumSamplesInFunction());
      painter.drawText(bounding_box, Qt::AlignRight, function_percetange_string);
      current += WidthPercentageColumn() + WidthMarginBetweenColumns();
    }

    // Draw total percentage
    {
      const QRect bounding_box{current, top_of(block), WidthPercentageColumn(),
                               fontMetrics().height()};
      painter.setPen(kLineNumberForegroundColor);

      QString total_percetange_string =
          FractionToPercentageString(num_samples_in_line, code_report_->GetNumSamples());
      painter.drawText(bounding_box, Qt::AlignRight, total_percetange_string);
      current += WidthPercentageColumn() + right_margin_.ToPixels(fontMetrics());
    }
  }
}

void Viewer::UpdateBarsSize() {
  const int number_of_lines = blockCount();

  int top_font_height = fontMetrics().height();

  int overall_left_width_px = heatmap_bar_width_.ToPixels(fontMetrics());

  if (line_numbers_enabled_) {
    overall_left_width_px += left_margin_.ToPixels(fontMetrics());
    overall_left_width_px += DetermineLineNumberWidthInPixels(fontMetrics(), number_of_lines);
    overall_left_width_px += right_margin_.ToPixels(fontMetrics());
  }
  int overall_right_width_px = 0;
  if (sample_counters_enabled_) {
    overall_right_width_px += left_margin_.ToPixels(fontMetrics());
    // Samples column
    overall_right_width_px += WidthSampleCounterColumn();
    overall_right_width_px += WidthMarginBetweenColumns();
    // Function Column
    overall_right_width_px += WidthPercentageColumn();
    overall_right_width_px += WidthMarginBetweenColumns();
    // Total Column
    overall_right_width_px += WidthPercentageColumn();
    overall_right_width_px += right_margin_.ToPixels(fontMetrics());
  }
  setViewportMargins(QMargins{overall_left_width_px, top_font_height, overall_right_width_px, 0});
  top_bar_widget_.SetSizeHint({contentsRect().width(), fontMetrics().height()});
  left_sidebar_widget_.SetSizeHint({overall_left_width_px, 0});
  right_sidebar_widget_.SetSizeHint({overall_right_width_px, 0});
}

void Viewer::UpdateBarsPosition() {
  auto top_bar = contentsRect();
  int total_width_without_scroll_bar = top_bar.width() - verticalScrollBar()->width();
  top_bar.setWidth(total_width_without_scroll_bar);
  top_bar.setHeight(fontMetrics().height());
  top_bar_widget_.setGeometry(top_bar);

  auto left_sidebar = contentsRect();
  left_sidebar.setTop(TopWidgetHeight());
  left_sidebar.setWidth(left_sidebar_widget_.sizeHint().width());
  left_sidebar_widget_.setGeometry(left_sidebar);

  auto right_sidebar = contentsRect();
  int right_sidebar_width = right_sidebar_widget_.sizeHint().width();
  right_sidebar.setTop(TopWidgetHeight());
  right_sidebar.moveLeft(total_width_without_scroll_bar - right_sidebar_width);
  right_sidebar.setWidth(right_sidebar_width);
  right_sidebar_widget_.setGeometry(right_sidebar);
}

void Viewer::SetEnableLineNumbers(bool is_enabled) {
  if (line_numbers_enabled_ == is_enabled) return;

  line_numbers_enabled_ = is_enabled;
  UpdateBarsSize();
}

void Viewer::SetEnableSampleCounters(bool is_enabled) {
  if (sample_counters_enabled_ == is_enabled) return;

  sample_counters_enabled_ = is_enabled;
  UpdateBarsSize();
}

void Viewer::SetLineNumberMargins(FontSizeInEm left, FontSizeInEm right) {
  left_margin_ = left;
  right_margin_ = right;
  UpdateBarsSize();
}

void Viewer::SetHeatmapBarWidth(FontSizeInEm width) {
  heatmap_bar_width_ = width;
  UpdateBarsSize();
}

void Viewer::SetHeatmapSource(const CodeReport* code_report) {
  code_report_ = code_report;
  UpdateBarsSize();
}

void Viewer::ClearHeatmapSource() {
  code_report_ = nullptr;
  UpdateBarsSize();
}

void Viewer::SetHighlightCurrentLine(bool enabled) {
  if (is_current_line_highlighted_ == enabled) return;

  is_current_line_highlighted_ = enabled;

  if (!is_current_line_highlighted_) {
    QObject::disconnect(this, &Viewer::cursorPositionChanged, this, &Viewer::HighlightCurrentLine);
    return;
  }

  QObject::connect(this, &Viewer::cursorPositionChanged, this, &Viewer::HighlightCurrentLine,
                   Qt::UniqueConnection);
  HighlightCurrentLine();
}

bool Viewer::IsCurrentLineHighlighted() const { return is_current_line_highlighted_; }

void Viewer::HighlightCurrentLine() {
  const QColor highlight_color = palette().base().color().lighter();

  QTextEdit::ExtraSelection selection{};
  selection.format.setBackground(highlight_color);
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = textCursor();
  selection.cursor.clearSelection();

  setExtraSelections({selection});
}

int Viewer::WidthPercentageColumn() const {
  const QString kWidestPercentage = "100.00 %";
  return StringWidthInPixels(fontMetrics(), kWidestPercentage);
}

int Viewer::WidthSampleCounterColumn() const {
  const QString kSampleColumnTitle = "Samples";
  return StringWidthInPixels(fontMetrics(), kSampleColumnTitle);
}

int Viewer::WidthMarginBetweenColumns() const {
  const QString kTwoSpaces = "  ";
  return StringWidthInPixels(fontMetrics(), kTwoSpaces);
}

int Viewer::TopWidgetHeight() const { return fontMetrics().height(); }

}  // namespace orbit_code_viewer