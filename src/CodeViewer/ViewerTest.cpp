// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QTextBlock>
#include <QTextDocument>
#include <array>
#include <limits>

#include "CodeViewer/Viewer.h"
#include "gtest/gtest.h"

namespace orbit_code_viewer {

static int argc = 0;

TEST(Viewer, DetermineLineNumberWidthInPixels) {
  QApplication app{argc, nullptr};

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

  (void)app;
}

TEST(Viewer, SetAnnotatingContentInDocumentEmpty) {
  QTextDocument doc{};
  constexpr int kNumberOfLines = 3;
  doc.setPlainText("first line\nsecond line\nthird line");
  ASSERT_EQ(doc.blockCount(), kNumberOfLines);

  const LargestOccurringLineNumbers max_line_numbers =
      SetAnnotatingContentInDocument(&doc, absl::Span<const AnnotatingLine>{});

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

  std::array<AnnotatingLine, 1> lines{};
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

  std::array<AnnotatingLine, 2> lines{};
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

  std::array<AnnotatingLine, 1> lines{};
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

  std::array<AnnotatingLine, 1> lines{};
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

  std::array<AnnotatingLine, 1> lines{};
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

  std::array<AnnotatingLine, 1> lines{};
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
}  // namespace orbit_code_viewer