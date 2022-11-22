// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_VIEWER_VIEWER_H_
#define CODE_VIEWER_VIEWER_H_

#include <absl/types/span.h>

#include <QFontMetrics>
#include <QObject>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QPointer>
#include <QResizeEvent>
#include <QString>
#include <QTextDocument>
#include <QWheelEvent>
#include <QWidget>
#include <cstdint>
#include <functional>
#include <optional>

#include "CodeReport/AnnotatingLine.h"
#include "CodeReport/CodeReport.h"
#include "CodeViewer/FontSizeInEm.h"
#include "CodeViewer/PlaceHolderWidget.h"

namespace orbit_code_viewer {

struct LargestOccurringLineNumbers {
  std::optional<uint64_t> main_content;
  std::optional<uint64_t> annotating_lines;
};

/*
  Viewer is a for displaying source code. It derives from a QPlainTextEdit
  and adds some additional features like a left sidebar for displaying
  line numbers and a heatmap indicator.

  Example usage:
  Viewer viewer{};
  viewer.SetEnableLineNumbers(true);

  // Add some spacing left and right of the line number
  viewer.SetLineNumbersMargins(Em{1.0}, Em{0.3});

  // The heatmap indicator area will be 2em wide.
  viewer.SetHeatmapBarWidth(Em{2.0});

  viewer.SetHeatmapSource([](int line_number) {
    (void) line_number;

    // The number we return need to be between 0.0f and 1.0f.
    return 0.5f;
  });

  viewer.show();

  Also check out the documentation of QPlainTextEdit for more details on how to
  use it.
*/
class Viewer : public QPlainTextEdit {
  Q_OBJECT
 public:
  explicit Viewer(QWidget* parent = nullptr);

  enum class LineNumberTypes { kNone, kOnlyMainContent, kOnlyAnnotatingLines, kBoth };
  void SetLineNumberTypes(LineNumberTypes line_number_types);
  void SetEnableSampleCounters(bool is_enabled);
  void SetLineNumberMargins(FontSizeInEm left, FontSizeInEm right);

  void SetHeatmapBarWidth(FontSizeInEm width);

  void SetHeatmapSource(const orbit_code_report::CodeReport* code_report);
  void ClearHeatmapSource();

  void SetHighlightCurrentLine(bool is_enabled);
  [[nodiscard]] bool IsCurrentLineHighlighted() const;

  void SetAnnotatingContent(absl::Span<const orbit_code_report::AnnotatingLine> annotating_lines);

  void SetTopBarTitle(const QString& title) { top_bar_title_ = title; }
  [[nodiscard]] const QString& GetTopBarTitle() const { return top_bar_title_; }

 private:
  void resizeEvent(QResizeEvent* ev) override;
  void wheelEvent(QWheelEvent* ev) override;
  void DrawTopWidget(QPaintEvent* event);
  void DrawLineNumbers(QPaintEvent* event);
  void DrawSampleCounters(QPaintEvent* event);

  void UpdateBarsSize();
  void UpdateBarsPosition();
  void HighlightCurrentLine();
  [[nodiscard]] int WidthPercentageColumn() const;
  [[nodiscard]] int WidthSampleCounterColumn() const;
  [[nodiscard]] int WidthMarginBetweenColumns() const;
  [[nodiscard]] int TopWidgetHeight() const;

  PlaceHolderWidget top_bar_widget_;
  PlaceHolderWidget left_sidebar_widget_;
  PlaceHolderWidget right_sidebar_widget_;
  LineNumberTypes line_number_types_ = LineNumberTypes::kNone;
  bool sample_counters_enabled_ = false;
  FontSizeInEm left_margin_ = FontSizeInEm{0.3f};
  FontSizeInEm right_margin_ = FontSizeInEm{0.3f};

  FontSizeInEm heatmap_bar_width_ = FontSizeInEm{0.0f};
  const orbit_code_report::CodeReport* code_report_ = nullptr;

  bool is_current_line_highlighted_ = false;

  // These are only used when SetAnnotatingContent was called and the line numbers deviate from
  // simple counting.
  LargestOccurringLineNumbers largest_occuring_line_numbers_;

  [[nodiscard]] uint64_t LargestOccurringLineNumber() const;

  QString top_bar_title_;
};

// Determine how many pixels are needed to draw all possible line numbers for the given font
[[nodiscard]] int DetermineLineNumberWidthInPixels(const QFontMetrics& font_metrics,
                                                   int max_line_number);

// Add a list of annotating lines to a document. The list of annotating lines need to be ordered by
// `.reference_line`.
[[nodiscard]] LargestOccurringLineNumbers SetAnnotatingContentInDocument(
    QTextDocument* document, absl::Span<const orbit_code_report::AnnotatingLine> annotating_lines);
}  // namespace orbit_code_viewer

#endif  // CODE_VIEWER_VIEWER_H_