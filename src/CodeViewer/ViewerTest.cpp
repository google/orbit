// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QSize>
#include <QString>
#include <QTextBlock>
#include <QTextDocument>
#include <QWheelEvent>
#include <Qt>
#include <array>
#include <optional>
#include <string>
#include <string_view>

#include "CodeReport/AnnotatingLine.h"
#include "CodeViewer/Viewer.h"

namespace orbit_code_viewer {

TEST(Viewer, DetermineLineNumberWidthInPixels) {
  const QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  const QFontMetrics font_metrics{font};

  const std::array max_line_numbers = {20, 99, 333, 999, 2000};

  for (const int max_line_number : max_line_numbers) {
    const int number_of_pixels_needed =
        DetermineLineNumberWidthInPixels(font_metrics, max_line_number);

    // All the line numbers between 1 and <max_line_number> need to fit
    // in the width calculated by DetermineLineNumberWidthInPixels.
    for (int line_number = 1; line_number <= max_line_number; ++line_number) {
      const auto line_number_label = QString::number(line_number);
      EXPECT_LE(font_metrics.horizontalAdvance(line_number_label), number_of_pixels_needed);
    }
  }
}

TEST(Viewer, SetAnnotatingContentInDocumentEmpty) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  const LargestOccurringLineNumbers max_line_numbers =
      SetAnnotatingContentInDocument(&doc, absl::Span<const orbit_code_report::AnnotatingLine>{});

  ASSERT_TRUE(max_line_numbers.main_content.has_value());
  ASSERT_EQ(max_line_numbers.main_content.value(), 3);

  ASSERT_FALSE(max_line_numbers.annotating_lines.has_value());
  EXPECT_EQ(doc.blockCount(), kNumberOfLines);

  for (auto current_block = doc.begin(); current_block != doc.end();
       current_block = current_block.next()) {
    EXPECT_NE(current_block.userData(), nullptr);
  }
}

TEST(Viewer, SetAnnotatingContentInDocumentFirst) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  std::array<orbit_code_report::AnnotatingLine, 1> lines{};
  lines[0].reference_line = 1;
  lines[0].line_contents = "first annotation";
  lines[0].line_number = 42;

  const LargestOccurringLineNumbers max_line_numbers = SetAnnotatingContentInDocument(&doc, lines);

  ASSERT_TRUE(max_line_numbers.main_content.has_value());
  ASSERT_EQ(max_line_numbers.main_content.value(), kNumberOfLines);

  ASSERT_TRUE(max_line_numbers.annotating_lines.has_value());
  ASSERT_EQ(max_line_numbers.annotating_lines.value(), 42);

  for (auto current_block = doc.begin(); current_block != doc.end();
       current_block = current_block.next()) {
    EXPECT_NE(current_block.userData(), nullptr);
  }

  EXPECT_EQ(doc.begin().text().toStdString(), "first annotation");
}

TEST(Viewer, SetAnnotatingContentInDocumentConsecutive) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  std::array<orbit_code_report::AnnotatingLine, 2> lines{};
  lines[0].reference_line = 1;
  lines[0].line_contents = "first annotation";
  lines[0].line_number = 42;
  lines[1].reference_line = 2;
  lines[1].line_contents = "second annotation";
  lines[1].line_number = 43;

  const LargestOccurringLineNumbers max_line_numbers = SetAnnotatingContentInDocument(&doc, lines);

  ASSERT_TRUE(max_line_numbers.main_content.has_value());
  ASSERT_EQ(max_line_numbers.main_content.value(), kNumberOfLines);

  ASSERT_TRUE(max_line_numbers.annotating_lines.has_value());
  ASSERT_EQ(max_line_numbers.annotating_lines.value(), 43);

  EXPECT_EQ(doc.blockCount(), 5);

  for (auto current_block = doc.begin(); current_block != doc.end();
       current_block = current_block.next()) {
    EXPECT_NE(current_block.userData(), nullptr);
  }

  EXPECT_EQ(doc.begin().text().toStdString(), "first annotation");
  EXPECT_EQ(doc.begin().next().text().toStdString(), "first line");
  EXPECT_EQ(doc.begin().next().next().text().toStdString(), "second annotation");
  EXPECT_EQ(doc.begin().next().next().next().text().toStdString(), "second line");
  EXPECT_EQ(doc.begin().next().next().next().next().text().toStdString(), "third line");
}

TEST(Viewer, SetAnnotatingContentInDocumentSecond) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  std::array<orbit_code_report::AnnotatingLine, 1> lines{};
  lines[0].reference_line = 2;
  lines[0].line_contents = "first annotation";
  lines[0].line_number = 42;

  const LargestOccurringLineNumbers max_line_numbers = SetAnnotatingContentInDocument(&doc, lines);

  ASSERT_TRUE(max_line_numbers.main_content.has_value());
  ASSERT_EQ(max_line_numbers.main_content.value(), 3);

  ASSERT_TRUE(max_line_numbers.annotating_lines.has_value());
  ASSERT_EQ(max_line_numbers.annotating_lines.value(), 42);

  for (auto current_block = doc.begin(); current_block != doc.end();
       current_block = current_block.next()) {
    EXPECT_NE(current_block.userData(), nullptr);
  }

  EXPECT_EQ(doc.begin().text().toStdString(), "first line");
  EXPECT_EQ(doc.begin().next().text().toStdString(), "first annotation");
  EXPECT_EQ(doc.begin().next().next().text().toStdString(), "second line");
}

TEST(Viewer, SetAnnotatingContentInDocumentLast) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  std::array<orbit_code_report::AnnotatingLine, 1> lines{};
  lines[0].reference_line = 3;
  lines[0].line_contents = "first annotation";
  lines[0].line_number = 42;

  const LargestOccurringLineNumbers max_line_numbers = SetAnnotatingContentInDocument(&doc, lines);

  ASSERT_TRUE(max_line_numbers.main_content.has_value());
  ASSERT_EQ(max_line_numbers.main_content.value(), kNumberOfLines);

  ASSERT_TRUE(max_line_numbers.annotating_lines.has_value());
  ASSERT_EQ(max_line_numbers.annotating_lines.value(), 42);

  for (auto current_block = doc.begin(); current_block != doc.end();
       current_block = current_block.next()) {
    EXPECT_NE(current_block.userData(), nullptr);
  }

  EXPECT_EQ(doc.begin().text().toStdString(), "first line");
  EXPECT_EQ(doc.begin().next().text().toStdString(), "second line");
  EXPECT_EQ(doc.begin().next().next().text().toStdString(), "first annotation");
  EXPECT_EQ(doc.begin().next().next().next().text().toStdString(), "third line");
}

TEST(Viewer, SetAnnotatingContentInDocumentInvalid) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  std::array<orbit_code_report::AnnotatingLine, 1> lines{};
  lines[0].reference_line = 4;  // Does not exist
  lines[0].line_contents = "first annotation";
  lines[0].line_number = 42;

  const LargestOccurringLineNumbers max_line_numbers = SetAnnotatingContentInDocument(&doc, lines);

  ASSERT_TRUE(max_line_numbers.main_content.has_value());
  ASSERT_EQ(max_line_numbers.main_content.value(), kNumberOfLines);

  ASSERT_FALSE(max_line_numbers.annotating_lines.has_value());

  for (auto current_block = doc.begin(); current_block != doc.end();
       current_block = current_block.next()) {
    EXPECT_NE(current_block.userData(), nullptr);
  }

  EXPECT_EQ(doc.begin().text().toStdString(), "first line");
  EXPECT_EQ(doc.begin().next().text().toStdString(), "second line");
  EXPECT_EQ(doc.begin().next().next().text().toStdString(), "third line");
}

TEST(Viewer, SetAnnotatingContentInDocumentFirstTwice) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  std::array<orbit_code_report::AnnotatingLine, 1> lines{};
  lines[0].reference_line = 1;
  lines[0].line_contents = "first annotation";
  lines[0].line_number = 42;

  {
    const LargestOccurringLineNumbers max_line_numbers =
        SetAnnotatingContentInDocument(&doc, lines);

    ASSERT_TRUE(max_line_numbers.main_content.has_value());
    ASSERT_EQ(max_line_numbers.main_content.value(), kNumberOfLines);

    ASSERT_TRUE(max_line_numbers.annotating_lines.has_value());
    ASSERT_EQ(max_line_numbers.annotating_lines.value(), 42);

    for (auto current_block = doc.begin(); current_block != doc.end();
         current_block = current_block.next()) {
      EXPECT_NE(current_block.userData(), nullptr);
    }

    EXPECT_EQ(doc.begin().text().toStdString(), "first annotation");
    EXPECT_EQ(doc.begin().next().text().toStdString(), "first line");
  }

  {
    const LargestOccurringLineNumbers max_line_numbers =
        SetAnnotatingContentInDocument(&doc, lines);

    ASSERT_TRUE(max_line_numbers.main_content.has_value());
    ASSERT_EQ(max_line_numbers.main_content.value(), kNumberOfLines);

    ASSERT_TRUE(max_line_numbers.annotating_lines.has_value());
    ASSERT_EQ(max_line_numbers.annotating_lines.value(), 42);

    for (auto current_block = doc.begin(); current_block != doc.end();
         current_block = current_block.next()) {
      EXPECT_NE(current_block.userData(), nullptr);
    }

    EXPECT_EQ(doc.begin().text().toStdString(), "first annotation");
    EXPECT_EQ(doc.begin().next().text().toStdString(), "first line");
  }
}

TEST(Viewer, Smoke) {
  // This is really just a smoke test. A lot of code is executed in the background but can't be
  // observed since it only affects the UI drawing. But at least we will know if that crashes due to
  // whatever reason.
  orbit_code_viewer::Viewer viewer{};
  viewer.show();

  QApplication::processEvents();

  constexpr std::string_view kContents{"first line\nsecond line\nthird line"};
  viewer.setPlainText(QString::fromLocal8Bit(kContents.data(), kContents.size()));
  EXPECT_EQ(viewer.toPlainText().toStdString(), kContents);

  QApplication::processEvents();

  // We also call `QWidget::setGeometry` to trigger the resize event which also calls a lot of extra
  // code. Again, this can't be really observed.
  const QSize new_size{800, 600};
  viewer.resize(new_size);
  EXPECT_EQ(viewer.geometry().size(), new_size);

  QApplication::processEvents();

  // We also send a wheel event to trigger the scaling code.
  QWheelEvent wheel_event{QPointF{viewer.geometry().center()},
                          QPointF{viewer.geometry().center()},
                          QPoint{},
                          QPoint{42, 42},
                          Qt::MouseButton::NoButton,
                          Qt::KeyboardModifier::NoModifier,
                          Qt::NoScrollPhase,
                          false};
  QApplication::sendEvent(&viewer, &wheel_event);

  QApplication::processEvents();
}

TEST(Viewer, HighlightCurrentLine) {
  orbit_code_viewer::Viewer viewer{};
  constexpr std::string_view kContents{"first line\nsecond line\nthird line"};
  viewer.setPlainText(QString::fromLocal8Bit(kContents.data(), kContents.size()));

  EXPECT_FALSE(viewer.IsCurrentLineHighlighted());

  viewer.SetHighlightCurrentLine(true);
  EXPECT_TRUE(viewer.IsCurrentLineHighlighted());

  viewer.SetHighlightCurrentLine(false);
  EXPECT_FALSE(viewer.IsCurrentLineHighlighted());
}
}  // namespace orbit_code_viewer