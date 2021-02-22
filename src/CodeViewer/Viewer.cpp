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
static const QColor kHeatmapColor{Qt::red};

int DetermineLineNumberWidthInPixels(const QFontMetrics& font_metrics, int max_line_number) {
  return font_metrics.horizontalAdvance(QString::number(max_line_number));
}

Viewer::Viewer(QWidget* parent) : QPlainTextEdit(parent), left_sidebar_widget_{this} {
  UpdateLeftSidebarWidth();
  QObject::connect(this, &QPlainTextEdit::blockCountChanged, this, &Viewer::UpdateLeftSidebarWidth);

  QObject::connect(&left_sidebar_widget_, &PlaceHolderWidget::PaintEventTriggered, this,
                   &Viewer::DrawLineNumbers);

  const auto update_viewport_area = [&](const QRect& rect, int dy) {
    const auto update_caused_by_scroll = [](int dy) { return dy != 0; };

    if (update_caused_by_scroll(dy)) {
      left_sidebar_widget_.scroll(0, dy);
    } else {
      QRect line_number_rect = rect;

      // We keep the (vertical) y-coordinates and adjust the horizontal x-coordinates for the line
      // number area.
      line_number_rect.setLeft(0);
      line_number_rect.setWidth(left_sidebar_widget_.sizeHint().width());
      left_sidebar_widget_.update(line_number_rect);
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

  auto left_sidebar = contentsRect();
  left_sidebar.setWidth(left_sidebar_widget_.sizeHint().width());
  left_sidebar_widget_.setGeometry(left_sidebar);
}

void Viewer::wheelEvent(QWheelEvent* ev) {
  QPlainTextEdit::wheelEvent(ev);

  UpdateLeftSidebarWidth();
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

    if (heatmap_bar_width_.Value() > 0.0 && heatmap_source_) {
      const QRect heatmap_rect{0, top_of(block), heatmap_bar_width_.ToPixels(fontMetrics()),
                               fontMetrics().height()};
      const float intensity = std::clamp(heatmap_source_(block.blockNumber()), 0.0f, 1.0f);
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

void Viewer::UpdateLeftSidebarWidth() {
  const int number_of_lines = blockCount();

  int overall_width_px = heatmap_bar_width_.ToPixels(fontMetrics());

  if (line_numbers_enabled_) {
    overall_width_px += left_margin_.ToPixels(fontMetrics());
    overall_width_px += DetermineLineNumberWidthInPixels(fontMetrics(), number_of_lines);
    overall_width_px += right_margin_.ToPixels(fontMetrics());
  }

  setViewportMargins(QMargins{overall_width_px, 0, 0, 0});
  left_sidebar_widget_.SetSizeHint({overall_width_px, 0});
}

void Viewer::SetEnableLineNumbers(bool is_enabled) {
  if (line_numbers_enabled_ == is_enabled) return;

  line_numbers_enabled_ = is_enabled;
  UpdateLeftSidebarWidth();
}

void Viewer::SetLineNumberMargins(FontSizeInEm left, FontSizeInEm right) {
  left_margin_ = left;
  right_margin_ = right;
  UpdateLeftSidebarWidth();
}

void Viewer::SetHeatmapBarWidth(FontSizeInEm width) {
  heatmap_bar_width_ = width;
  UpdateLeftSidebarWidth();
}

void Viewer::SetHeatmapSource(HeatmapSource heatmap_source) {
  heatmap_source_ = std::move(heatmap_source);
  UpdateLeftSidebarWidth();
}

void Viewer::ClearHeatmapSource() {
  heatmap_source_ = {};
  UpdateLeftSidebarWidth();
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

}  // namespace orbit_code_viewer