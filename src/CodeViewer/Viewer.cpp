// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeViewer/Viewer.h"

#include <OrbitBase/Logging.h>

#include <QColor>
#include <QFont>
#include <QGradientStops>
#include <QMargins>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QPlainTextEdit>
#include <QRect>
#include <QRectF>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSize>
#include <QString>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFormat>
#include <QTextOption>
#include <QWheelEvent>
#include <Qt>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <memory>

#include "SyntaxHighlighter/HighlightingMetadata.h"

namespace orbit_code_viewer {

static const QColor kLineNumberBackgroundColor{50, 50, 50};
static const QColor kLineNumberForegroundColor{189, 189, 189};
static const QColor kTextEditBackgroundColor{30, 30, 30};
static const QColor kAnnotatingLinesBackgroundColor{50, 50, 50};
static const QColor kTextEditForegroundColor{189, 189, 189};
static const QColor kTitleBackgroundColor{30, 65, 89};
static const QColor kHeatmapColor{Qt::red};

static int StringWidthInPixels(const QFontMetrics& font_metrics, const QString& string) {
  return font_metrics.horizontalAdvance(string);
}

int DetermineLineNumberWidthInPixels(const QFontMetrics& font_metrics, int max_line_number) {
  return StringWidthInPixels(font_metrics, QString::number(max_line_number));
}

namespace {
struct Metadata : public orbit_syntax_highlighter::HighlightingMetadata {
  enum LineType { kMainContent, kAnnotatingLine };

  LineType line_type = kMainContent;
  uint64_t line_number = 0ul;

  explicit Metadata(LineType line_type, uint64_t line_number)
      : line_type(line_type), line_number(line_number) {}
  explicit Metadata() = default;

  [[nodiscard]] bool IsMainContentLine() const override { return line_type == kMainContent; }
};
}  // namespace

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
  QFont document_default_font = document()->defaultFont();

  QPlainTextEdit::wheelEvent(ev);

  // QPlainTextEdit::wheelEvent updates the default font to adjust the size.
  // We respect the size change but stick to the user's default font.
  const QFont wrong_font_with_correct_size = document()->defaultFont();
  document_default_font.setPointSize(wrong_font_with_correct_size.pointSize());
  document()->setDefaultFont(document_default_font);

  UpdateBarsSize();
  UpdateBarsPosition();
}

void Viewer::DrawTopWidget(QPaintEvent* event) {
  QPainter painter{&top_bar_widget_};
  painter.setFont(font());
  painter.fillRect(event->rect(), kTitleBackgroundColor);

  if (line_number_types_ != LineNumberTypes::kNone) {
    const int left = (left_margin_ + heatmap_bar_width_).ToPixels(fontMetrics());
    const int width = DetermineLineNumberWidthInPixels(fontMetrics(), blockCount());
    const QRect bounding_box{left, 0, width, fontMetrics().height()};

    painter.setPen(kLineNumberForegroundColor);
    painter.drawText(bounding_box, Qt::AlignCenter, QString("#"));
  }

  {
    const int left = left_sidebar_widget_.width();
    const int width = top_bar_widget_.width() - left - right_sidebar_widget_.width();
    const QRect bounding_box{left, 0, width, fontMetrics().height()};

    painter.setPen(kLineNumberForegroundColor);
    painter.drawText(bounding_box, Qt::AlignLeft, top_bar_title_);
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

    // We will only show the heatmap bar for lines belonging to the main content - which means the
    // metadata's line type needs to be set to kMainContent. If no metadata is set we will assume
    // it's main content.
    const auto* const metadata = static_cast<const Metadata*>(block.userData());

    const auto line_number = [&]() -> uint64_t {
      if (metadata != nullptr) return metadata->line_number;
      return block.firstLineNumber() + 1;
    }();

    const bool is_main_content =
        metadata == nullptr || metadata->line_type == Metadata::kMainContent;

    if (heatmap_bar_width_.Value() > 0.0 && code_report_ != nullptr && is_main_content) {
      const QRect heatmap_rect{0, top_of(block), heatmap_bar_width_.ToPixels(fontMetrics()),
                               fontMetrics().height()};

      const uint32_t num_samples_in_line =
          code_report_->GetNumSamplesAtLine(line_number).value_or(0);
      const uint32_t num_samples_in_function = code_report_->GetNumSamplesInFunction();

      const int scaled_intensity = [&] {
        if (num_samples_in_function == 0) return 0;

        const float intensity = std::clamp(
            static_cast<float>(num_samples_in_line) / static_cast<float>(num_samples_in_function),
            0.0f, 1.0f);
        return static_cast<int>(std::sqrt(intensity) * 255);
      }();

      QColor color = kHeatmapColor;
      color.setAlpha(scaled_intensity);

      painter.fillRect(heatmap_rect, color);
    }

    const auto is_line_number_enabled = [&]() {
      if (line_number_types_ == LineNumberTypes::kBoth) return true;

      if (metadata == nullptr && line_number_types_ == LineNumberTypes::kOnlyMainContent) {
        return true;
      }
      if (metadata == nullptr) return false;

      if (metadata->line_type == Metadata::kMainContent &&
          line_number_types_ == LineNumberTypes::kOnlyMainContent) {
        return true;
      }
      if (metadata->line_type == Metadata::kAnnotatingLine &&
          line_number_types_ == LineNumberTypes::kOnlyAnnotatingLines) {
        return true;
      }

      return false;
    }();

    if (is_line_number_enabled) {
      const int left = (left_margin_ + heatmap_bar_width_).ToPixels(fontMetrics());
      const int width =
          DetermineLineNumberWidthInPixels(fontMetrics(), static_cast<int>(line_number));
      const QRect bounding_box{left, top_of(block), width, fontMetrics().height()};

      painter.setPen(kLineNumberForegroundColor);
      painter.drawText(bounding_box, Qt::AlignRight, QString::number(line_number));
    }
  }
}

// For example, from a value = 0.5, we want to print "50.00 %"
static QString FractionToPercentageString(int a, int b) {
  double ratio = (b == 0 ? 0. : static_cast<double>(a) / b);
  double percentage_ratio = std::clamp(ratio * 100., 0., 100.);
  return QString{"%1 %"}.arg(percentage_ratio, 0, 'f', 2);
}

void Viewer::DrawSampleCounters(QPaintEvent* event) {
  if (!sample_counters_enabled_) return;
  QPainter painter{&right_sidebar_widget_};
  painter.setFont(font());
  painter.fillRect(event->rect(), kLineNumberBackgroundColor);

  if (code_report_ == nullptr) return;

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

    const auto* const metadata = static_cast<const Metadata*>(block.userData());

    const bool is_main_content =
        metadata == nullptr || metadata->line_type == Metadata::kMainContent;

    if (!is_main_content) continue;

    const auto line_number = [&]() -> uint64_t {
      if (metadata == nullptr) return block.firstLineNumber() + 1;
      return metadata->line_number;
    }();

    auto maybe_num_samples_in_lines = code_report_->GetNumSamplesAtLine(line_number);
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

      QString function_percentage_string =
          FractionToPercentageString(num_samples_in_line, code_report_->GetNumSamplesInFunction());
      painter.drawText(bounding_box, Qt::AlignRight, function_percentage_string);
      current += WidthPercentageColumn() + WidthMarginBetweenColumns();
    }

    // Draw total percentage
    {
      const QRect bounding_box{current, top_of(block), WidthPercentageColumn(),
                               fontMetrics().height()};
      painter.setPen(kLineNumberForegroundColor);

      QString total_percentage_string =
          FractionToPercentageString(num_samples_in_line, code_report_->GetNumSamples());
      painter.drawText(bounding_box, Qt::AlignRight, total_percentage_string);
      current += WidthPercentageColumn() + right_margin_.ToPixels(fontMetrics());
    }
  }
}

uint64_t Viewer::LargestOccurringLineNumber() const {
  switch (line_number_types_) {
    case LineNumberTypes::kNone:
      return 0;
    case LineNumberTypes::kOnlyMainContent:
      return largest_occuring_line_numbers_.main_content.value_or(blockCount());
    case LineNumberTypes::kOnlyAnnotatingLines:
      return largest_occuring_line_numbers_.annotating_lines.value_or(0);
    case LineNumberTypes::kBoth:
      return std::max(largest_occuring_line_numbers_.main_content.value_or(blockCount()),
                      largest_occuring_line_numbers_.annotating_lines.value_or(0));
  }

  return 0;
}

void Viewer::UpdateBarsSize() {
  const auto number_of_lines = LargestOccurringLineNumber();

  int top_font_height = fontMetrics().height();

  int overall_left_width_px = heatmap_bar_width_.ToPixels(fontMetrics());

  if (line_number_types_ != LineNumberTypes::kNone) {
    overall_left_width_px += left_margin_.ToPixels(fontMetrics());
    overall_left_width_px +=
        DetermineLineNumberWidthInPixels(fontMetrics(), static_cast<int>(number_of_lines));
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

void Viewer::SetLineNumberTypes(LineNumberTypes line_number_types) {
  if (line_number_types_ == line_number_types) return;

  line_number_types_ = line_number_types;
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

void Viewer::SetHeatmapSource(const orbit_code_report::CodeReport* code_report) {
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
  QColor current_background_color = textCursor().block().blockFormat().background().color();
  if (current_background_color == Qt::black) {  // color hasn't been set in the block
    current_background_color = palette().base().color();
  }

  QTextEdit::ExtraSelection selection{};
  selection.format.setBackground(current_background_color.lighter());
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = textCursor();
  selection.cursor.clearSelection();

  setExtraSelections({selection});
}

int Viewer::WidthPercentageColumn() const {
  const QString widest_percentage = "100.00 %";
  return StringWidthInPixels(fontMetrics(), widest_percentage);
}

int Viewer::WidthSampleCounterColumn() const {
  const QString sample_column_title = "Samples";
  return StringWidthInPixels(fontMetrics(), sample_column_title);
}

int Viewer::WidthMarginBetweenColumns() const {
  const QString two_spaces = "  ";
  return StringWidthInPixels(fontMetrics(), two_spaces);
}

int Viewer::TopWidgetHeight() const { return fontMetrics().height(); }

void Viewer::SetAnnotatingContent(
    absl::Span<const orbit_code_report::AnnotatingLine> annotating_lines) {
  largest_occuring_line_numbers_ = SetAnnotatingContentInDocument(document(), annotating_lines);
  UpdateBarsSize();
}

LargestOccurringLineNumbers SetAnnotatingContentInDocument(
    QTextDocument* document, absl::Span<const orbit_code_report::AnnotatingLine> annotating_lines) {
  // Lets first go through the main content and save line numbers as metadata.
  // If previously extra annotating content had been added, let's remove it now.
  for (auto current_block = document->begin(); current_block != document->end();) {
    const auto* const metadata = static_cast<const Metadata*>(current_block.userData());

    if (metadata == nullptr) {
      auto user_data =
          std::make_unique<Metadata>(Metadata::kMainContent, current_block.firstLineNumber() + 1);

      // We transfer ownership
      current_block.setUserData(user_data.release());
      current_block = current_block.next();
      continue;
    }

    if (metadata->line_type == Metadata::kAnnotatingLine) {
      // This must be from previous calls of this function. Lets remove this line.
      QTextCursor cursor{current_block};
      current_block = current_block.next();
      cursor.select(QTextCursor::BlockUnderCursor);
      cursor.removeSelectedText();
      cursor.deleteChar();  // Deletes line break
      continue;
    }

    current_block = current_block.next();
  }

  LargestOccurringLineNumbers largest_occuring_line_numbers{};
  largest_occuring_line_numbers.main_content = document->blockCount();

  const auto* current_annotating_line = annotating_lines.begin();

  // In a second pass we add the annotating lines
  for (auto current_block = document->begin();
       current_block != document->end() && current_annotating_line != annotating_lines.end();
       current_block = current_block.next()) {
    const auto* const metadata = static_cast<const Metadata*>(current_block.userData());
    ORBIT_CHECK(metadata != nullptr);

    if (metadata->line_number == current_annotating_line->reference_line) {
      // That means the annotating line needs to go in front of the current block

      QTextCursor cursor{current_block};
      cursor.movePosition(QTextCursor::StartOfBlock);
      cursor.insertText(
          QString::fromStdString(current_annotating_line->line_contents).append('\n'));

      QTextCursor prev_cursor{cursor.block().previous()};
      QTextBlockFormat annotating_format;
      annotating_format.setBackground(kAnnotatingLinesBackgroundColor);
      prev_cursor.setBlockFormat(annotating_format);

      // We transfer ownership NOLINTNEXTLINE
      cursor.block().setUserData(new Metadata{});

      auto* previous_block_metadata = static_cast<Metadata*>(cursor.block().previous().userData());
      auto* current_block_metadata = static_cast<Metadata*>(cursor.block().userData());

      // When inserting a new block in front of an existing block the metadata does stay with the
      // new block, so we have to copy it to the old one here.
      *current_block_metadata = *previous_block_metadata;

      previous_block_metadata->line_type = Metadata::kAnnotatingLine;
      previous_block_metadata->line_number = current_annotating_line->line_number;

      largest_occuring_line_numbers.annotating_lines =
          std::max(current_annotating_line->line_number,
                   largest_occuring_line_numbers.annotating_lines.value_or(0));

      current_block = current_block.next();
      ++current_annotating_line;
    }
  }

  return largest_occuring_line_numbers;
}

}  // namespace orbit_code_viewer
