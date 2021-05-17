// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SyntaxHighlighter/Cpp.h"

#include <QColor>
#include <QRegularExpression>
#include <QString>
#include <QTextCharFormat>

namespace orbit_syntax_highlighter {
namespace {
// Colors are taken from Clion Darcula theme
const QColor kGrey{0x80, 0x80, 0x80};
const QColor kBlue{0x61, 0x96, 0xcc};
const QColor kYellow{0xa0, 0xa0, 0x33};
const QColor kYellowOrange{0xff, 0xcc, 0x66};
const QColor kOrange{0xcc, 0x66, 0x33};
const QColor kOlive{0x80, 0x80, 0x00};
const QColor kGreen{0x66, 0x99, 0x66};
const QColor kViolet{0x99, 0x66, 0x99};
const QColor kGreyViolet{0xcc, 0xaa, 0xcc};

enum CppHighlighterState { kInitialState, kOpenCommentState, kOpenStringState };
namespace CppRegex {
const char* const kNumberRegex =
    "(?:\\b0b[01']+|\\b0x(?:[\\da-f']+(?:\\.[\\da-f']*)?|\\.[\\da-f']+)(?:p[+-]?[\\d']+)?|(?:\\b["
    "\\d']+(?:\\.[\\d']*)?|\\B\\.[\\d']+)(?:e[+-]?[\\d']+)?)[ful]{0,4}";
const char* const kConstantRegex =
    "__FILE__|__LINE__|__DATE__|__TIME__|__TIMESTAMP__|__func__|EOF|NULL|SEEK_CUR|SEEK_END|SEEK_"
    "SET|stdin|stdout|stderr";
const char* const kKeywordRegex =
    "\\b(?:alignas|alignof|asm|auto|bool|break|case|catch|char|char8_t|char16_t|char32_t|class|"
    "compl|concept|const|consteval|constexpr|constinit|const_cast|continue|co_await|co_return|co_"
    "yield|decltype|default|delete|do|double|dynamic_cast|else|enum|explicit|export|extern|false|"
    "final|float|for|friend|goto|if|import|inline|int|int8_t|int16_t|int32_t|int64_t|uint8_t|"
    "uint16_t|uint32_t|uint64_t|long|mutable|namespace|new|noexcept|nullptr|operator|override|"
    "private|protected|public|register|reinterpret_cast|requires|return|short|signed|sizeof|static|"
    "static_|assert|static_cast|struct|switch|template|this|thread_local|throw|true|try|typedef|"
    "typeid|typename|union|unsigned|using|virtual|void|volatile|wchar_t|while)\\b";

const char* const kCapitalizedRegex = "(?<=[\\s\\(<])[A-Z][\\w]*";
// int Function( or Namespace::FunctionName( patterns
const char* const kFunctionDefinitionRegex = "(?<=\\w)\\s+([A-Za-z_]\\w*::)*[A-Za-z_]\\w*(?=\\()";
const char* const kOnlyUppercaseRegex = "(?<=[\\s\\(])[A-Z][0-9A-Z\\_]*(?=\\b)";
const char* const kCommaRegex = "[\\;\\,]";
// Methods and variables from a namespace, after "::" (e.g. std::cout, absl::Milliseconds)
const char* const kNamespaceVariablesRegex = "(?<=::)[A-Za-z_]\\w*(?=\\b)";
// Namespaces itself, before "::"
const char* const kNamespaceRegex = "[A-Za-z_]\\w*::";
const char* const kClassNameRegex = "\\b(?:class|concept|enum|namespace|struct|typename)\\s+(\\w+)";
// Variables starting with lowercase and finishing with _ (e.g. tracks_) or starting with "m_"
const char* const kClassMemberRegex = "\\b[a-z]\\w*\\_(?<=\\b)|(?<=\\b)m_\\w*";
const char* const kPreprocessorRegex = "(^\\s*)#\\s*[A-Za-z_]\\w*";
// Match with <word> after #include
const char* const kIncludeFileRegex = "(?<=#include)\\s*<[^>]*>";
const char* const kCommentRegex =
    "\\/\\/(?:[^\\r\\n\\\\]|\\\\(?:\\r\\n?|\\n|(?![\\r\\n])))*|\\/\\*[\\s\\S]*?\\*\\/";
// Match with "/*" comments which starts but not finishes in this line
const char* const kOpenCommentRegex = "\\/\\*([^\\*]|[\\*]+[^\\/])*?$";
// Match with the closing part of the comment
const char* const kEndCommentRegex = "[\\s\\S]*\\*\\/";
// Match with a line without a closing comment
const char* const kNoEndCommentRegex = "([^\\*]|\\*+[^\\/\\*])*$";
// Similar process to comments to match a multiline string
const char* const kStringRegex = "\"([^\\\\\"]|\\\\.)*\"|\'[^\']*\'";
const char* const kOpenStringRegex = "\"([^\\\\\"]|\\\\.)*\\\\$";
const char* const kEndStringRegex = "([^\\\\\"]|\\\\.)*\"";
const char* const kNoEndStringRegex = "([^\\\\\"]|\\\\.)*\\\\$";
}  // namespace CppRegex
}  // namespace

Cpp::Cpp() : QSyntaxHighlighter{static_cast<QObject*>(nullptr)} {
  using PatternOption = QRegularExpression::PatternOption;
  number_regex_ = QRegularExpression{CppRegex::kNumberRegex, PatternOption::CaseInsensitiveOption};
  constant_regex_ = QRegularExpression{CppRegex::kConstantRegex};
  keyword_regex_ = QRegularExpression{CppRegex::kKeywordRegex};
  comment_regex_ = QRegularExpression{CppRegex::kCommentRegex};
  open_comment_regex_ = QRegularExpression{CppRegex::kOpenCommentRegex};
  end_comment_regex_ = QRegularExpression{CppRegex::kEndCommentRegex};
  no_end_comment_regex_ = QRegularExpression{CppRegex::kNoEndCommentRegex};
  function_definition_regex_ =
      QRegularExpression{CppRegex::kFunctionDefinitionRegex, PatternOption::CaseInsensitiveOption};
  preprocessor_regex_ = QRegularExpression{CppRegex::kPreprocessorRegex};
  include_file_regex_ = QRegularExpression{CppRegex::kIncludeFileRegex};
  string_regex_ = QRegularExpression{CppRegex::kStringRegex};
  open_string_regex_ = QRegularExpression{CppRegex::kOpenStringRegex};
  end_string_regex_ = QRegularExpression{CppRegex::kEndStringRegex};
  no_end_string_regex_ = QRegularExpression{CppRegex::kNoEndStringRegex};
  comma_regex_ = QRegularExpression{CppRegex::kCommaRegex};
  capitalized_regex_ = QRegularExpression{CppRegex::kCapitalizedRegex};
  only_uppercase_regex_ = QRegularExpression{CppRegex::kOnlyUppercaseRegex};
  namespace_regex_ = QRegularExpression{CppRegex::kNamespaceRegex};
  namespace_variables_regex_ = QRegularExpression{CppRegex::kNamespaceVariablesRegex};
  class_name_regex_ = QRegularExpression{CppRegex::kClassNameRegex};
  class_member_regex_ = QRegularExpression{CppRegex::kClassMemberRegex};
}

CppHighlighterState HighlightBlockCpp(
    const QString& code, CppHighlighterState previous_block_state,
    std::function<void(int, int, const QTextCharFormat&)> set_format) {
  CppHighlighterState next_block_state = CppHighlighterState::kInitialState;
  const auto apply = [&code, &set_format, &next_block_state](
                         QRegularExpression* expression, const QColor& color,
                         CppHighlighterState new_state = CppHighlighterState::kInitialState) {
    QTextCharFormat format{};
    format.setForeground(color);

    for (auto it = expression->globalMatch(code); it.hasNext();) {
      const auto match = it.next();
      set_format(match.capturedStart(), match.capturedLength(), format);
      next_block_state = new_state;
    }
  };

  // We are processing line by line and trying to find all substrings that match with these
  // patterns. As each one paints over the others, order matters.

  // Ordered heuristics for painting certain word patterns. Should be at the first.
  apply(&Cpp::capitalized_regex_, kGreyViolet);
  apply(&Cpp::namespace_variables_regex_, kGreyViolet);
  apply(&Cpp::function_definition_regex_, kYellowOrange);
  apply(&Cpp::namespace_regex_, kGreyViolet);
  apply(&Cpp::only_uppercase_regex_, kOlive);

  // Extra patterns which make the syntax highlighter nicer. Order doesn't matter.
  apply(&Cpp::class_name_regex_, kGreyViolet);
  apply(&Cpp::number_regex_, kBlue);
  apply(&Cpp::class_member_regex_, kViolet);
  apply(&Cpp::comma_regex_, kOrange);

  // Special C/C++ patterns. Order doesn't matter.
  apply(&Cpp::keyword_regex_, kOrange);
  apply(&Cpp::constant_regex_, kOlive);
  apply(&Cpp::include_file_regex_, kGreen);
  apply(&Cpp::preprocessor_regex_, kYellow);

  // Comments and strings should be painted at the end.
  apply(&Cpp::string_regex_, kGreen);
  apply(&Cpp::comment_regex_, kGrey);

  // For several-lines comments and strings, we have these states.
  if (previous_block_state == CppHighlighterState::kOpenStringState) {
    apply(&Cpp::no_end_string_regex_, kGreen, CppHighlighterState::kOpenStringState);
    apply(&Cpp::end_string_regex_, kGreen);
  }
  if (previous_block_state == CppHighlighterState::kOpenCommentState) {
    apply(&Cpp::no_end_comment_regex_, kGrey, CppHighlighterState::kOpenCommentState);
    apply(&Cpp::end_comment_regex_, kGrey);
  }
  apply(&Cpp::open_string_regex_, kGreen, CppHighlighterState::kOpenStringState);
  apply(&Cpp::open_comment_regex_, kGrey, CppHighlighterState::kOpenCommentState);
}

void Cpp::highlightBlock(const QString& code) {
  const auto apply = [&code, this](
                         QRegularExpression* expression, const QColor& color,
                         CppHighlighterState new_state = CppHighlighterState::kInitialState) {
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

  // Ordered heuristics for painting certain word patterns. Should be at the first.
  apply(&capitalized_regex_, kGreyViolet);
  apply(&namespace_variables_regex_, kGreyViolet);
  apply(&function_definition_regex_, kYellowOrange);
  apply(&namespace_regex_, kGreyViolet);
  apply(&only_uppercase_regex_, kOlive);

  // Extra patterns which make the syntax highlighter nicer. Order doesn't matter.
  apply(&class_name_regex_, kGreyViolet);
  apply(&number_regex_, kBlue);
  apply(&class_member_regex_, kViolet);
  apply(&comma_regex_, kOrange);

  // Special C/C++ patterns. Order doesn't matter.
  apply(&keyword_regex_, kOrange);
  apply(&constant_regex_, kOlive);
  apply(&include_file_regex_, kGreen);
  apply(&preprocessor_regex_, kYellow);

  // Comments and strings should be painted at the end.
  apply(&string_regex_, kGreen);
  apply(&comment_regex_, kGrey);

  // For several-lines comments and strings, we have these states.
  if (previousBlockState() == CppHighlighterState::kOpenStringState) {
    apply(&no_end_string_regex_, kGreen, CppHighlighterState::kOpenStringState);
    apply(&end_string_regex_, kGreen);
  }
  if (previousBlockState() == CppHighlighterState::kOpenCommentState) {
    apply(&no_end_comment_regex_, kGrey, CppHighlighterState::kOpenCommentState);
    apply(&end_comment_regex_, kGrey);
  }
  apply(&open_string_regex_, kGreen, CppHighlighterState::kOpenStringState);
  apply(&open_comment_regex_, kGrey, CppHighlighterState::kOpenCommentState);
}
}  // namespace orbit_syntax_highlighter
