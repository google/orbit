// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SyntaxHighlighter/Cpp.h"

#include <QColor>
#include <QRegularExpression>
#include <QString>
#include <QTextCharFormat>

namespace orbit_syntax_highlighter {
namespace C {
namespace {
enum HighlighterState { kInitialState, kOpenCommentState, kOpenStringState };

const QColor kGrey{0x80, 0x80, 0x80};
const QColor kBlue{0x61, 0x96, 0xcc};
const QColor kYellow{0xa0, 0xa0, 0x33};
const QColor kYellowOrange{0xff, 0xcc, 0x66};
const QColor kOrange{0xcc, 0x66, 0x33};
const QColor kOlive{0x80, 0x80, 0x00};
const QColor kGreen{0x66, 0x99, 0x66};
const QColor kViolet{0xcc, 0x99, 0xcd};

const char* const kNumberRegex =
    "(?:\\b0x(?:[\\da-f]+(?:\\.[\\da-f]*)?|\\.[\\da-f]+)(?:p[+-]?\\d+)?|(?:\\b\\d+(?:\\.\\d*)?|"
    "\\B\\.\\d+)(?:e[+-]?\\d+)?)[ful]{0,4}";
const char* const kConstantRegex =
    "__FILE__|__LINE__|__DATE__|__TIME__|__TIMESTAMP__|__func__|EOF|NULL|SEEK_CUR|SEEK_END|SEEK_"
    "SET|stdin|stdout|stderr";
const char* const kKeywordRegex =
    "\\b(?:__attribute__|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Generic|_Imaginary|_Noreturn|_"
    "Static_assert|_Thread_local|asm|typeof|inline|auto|break|case|char|const|continue|default|do|"
    "double|else|enum|extern|float|for|goto|if|int|long|register|return|short|signed|sizeof|static|"
    "struct|switch|typedef|union|unsigned|void|volatile|while|)\\b";
// Match with comments starting and ending in the same line
const char* const kCommentRegex =
    "\\/\\/(?:[^\\r\\n\\\\]|\\\\(?:\\r\\n?|\\n|(?![\\r\\n])))*|\\/\\*[\\s\\S]*?\\*\\/";
// Match with "/*" comments which starts but not finishes in this line
const char* const kOpenCommentRegex = "\\/\\*([^\\*]|[\\*]+[^\\/])*?$";
// Match with the closing part of the comment
const char* const kEndCommentRegex = "[\\s\\S]*\\*\\/";
// Match with a line without a closing comment
const char* const kNoEndCommentRegex = "([^\\*]|\\*+[^\\/\\*])*$";
// A word who is followed with an open-parenthesis (
const char* const kFunctionRegex = "(?<=[^\\(\\:\\.\\w\\>])[a-z_]\\w*(?=\\s*\\()";
const char* const kPreprocessorRegex = "(^\\s*)#\\s*[a-z_]\\w*";
// Match with <word> after #include
const char* const kIncludeFileRegex = "(?<=#include)\\s*<[^>]+>";
// Similar process to comments to match a multiline string
const char* const kStringRegex = "\"([^\\\\\"]|\\\\.)*\"|\'[^\']*\'";
const char* const kOpenStringRegex = "\"([^\\\\\"]|\\\\.)*\\\\$";
const char* const kEndStringRegex = "([^\\\\\"]|\\\\.)*\"";
const char* const kNoEndStringRegex = "([^\\\\\"]|\\\\.)*\\\\$";
const char* const kCapitalizedRegex = "(?<=[\\s\\(<])[A-Z][\\w\\*\\&]*";
const char* const kNoLowercaseRegex = "(?<=[\\s\\(])[A-Z][0-9A-Z\\_\\*\\&]*(?=[\\;\\,\\(\\s\\)])";
}  // namespace
}  // namespace C

Cpp::Cpp() : QSyntaxHighlighter{static_cast<QObject*>(nullptr)} {
  using PatternOption = QRegularExpression::PatternOption;
  number_regex_ = QRegularExpression{C::kNumberRegex, PatternOption::CaseInsensitiveOption};
  constant_regex_ = QRegularExpression{C::kConstantRegex};
  keyword_regex_ = QRegularExpression{C::kKeywordRegex};
  comment_regex_ = QRegularExpression{C::kCommentRegex};
  open_comment_regex_ = QRegularExpression{C::kOpenCommentRegex};
  end_comment_regex_ = QRegularExpression{C::kEndCommentRegex};
  no_end_comment_regex_ = QRegularExpression{C::kNoEndCommentRegex};
  function_regex_ = QRegularExpression{C::kFunctionRegex, PatternOption::CaseInsensitiveOption};
  preprocessor_regex_ = QRegularExpression{C::kPreprocessorRegex};
  include_file_regex_ = QRegularExpression{C::kIncludeFileRegex};
  string_regex_ = QRegularExpression{C::kStringRegex};
  open_string_regex_ = QRegularExpression{C::kOpenStringRegex};
  end_string_regex_ = QRegularExpression{C::kEndStringRegex};
  no_end_string_regex_ = QRegularExpression{C::kNoEndStringRegex};
  capitalized_regex_ = QRegularExpression{C::kCapitalizedRegex};
  no_lowercase_regex_ = QRegularExpression{C::kNoLowercaseRegex};
}

void Cpp::highlightBlock(const QString& code) {
  const auto apply = [&code, this](
                         QRegularExpression* expression, const QColor& color,
                         C::HighlighterState new_state = C::HighlighterState::kInitialState) {
    QTextCharFormat format{};
    format.setForeground(color);

    for (auto it = expression->globalMatch(code); it.hasNext();) {
      const auto match = it.next();
      setFormat(match.capturedStart(), match.capturedLength(), format);
      setCurrentBlockState(new_state);
    }
  };

  // We are processing line by line and trying to find all substrings that match with these
  // patterns. As each one paints over the others, order matters.

  // Ordered heuristics for painting certain word patterns.
  apply(&capitalized_regex_, C::kViolet);
  apply(&function_regex_, C::kYellowOrange);
  apply(&no_lowercase_regex_, C::kOlive);

  apply(&number_regex_, C::kBlue);

  // Special C/C++ words: The order here doesn't matter.
  apply(&keyword_regex_, C::kOrange);
  apply(&constant_regex_, C::kOlive);
  apply(&include_file_regex_, C::kGreen);
  apply(&preprocessor_regex_, C::kYellow);

  // Comments and strings should be painted at the end.
  apply(&string_regex_, C::kGreen);
  apply(&comment_regex_, C::kGrey);

  // For several-lines comments and strings, we have these states.
  if (previousBlockState() == C::HighlighterState::kOpenStringState) {
    apply(&no_end_string_regex_, C::kGreen, C::HighlighterState::kOpenStringState);
    apply(&end_string_regex_, C::kGreen);
  }
  if (previousBlockState() == C::HighlighterState::kOpenCommentState) {
    apply(&no_end_comment_regex_, C::kGrey, C::HighlighterState::kOpenCommentState);
    apply(&end_comment_regex_, C::kGrey);
  }
  apply(&open_string_regex_, C::kGreen, C::HighlighterState::kOpenStringState);
  apply(&open_comment_regex_, C::kGrey, C::HighlighterState::kOpenCommentState);
}
}  // namespace orbit_syntax_highlighter
