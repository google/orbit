// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SyntaxHighlighter/Cpp.h"

#include <QColor>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
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

namespace CppRegex {
using PatternOption = QRegularExpression::PatternOption;

const QRegularExpression kNumberRegex{
    "((?:\\b0b[01']+|\\b0x(?:[\\da-f']+(?:\\.[\\da-f']*)?|\\.[\\da-f']+)(?:p[+-]?[\\d']+)?|(?:\\b["
    "\\d']+(?:\\.[\\d']*)?|\\B\\.[\\d']+)(?:e[+-]?[\\d']+)?)[ful]{0,4})",
    PatternOption::CaseInsensitiveOption};
const QRegularExpression kConstantRegex{
    "(__FILE__|__LINE__|__DATE__|__TIME__|__TIMESTAMP__|__func__|EOF|NULL|SEEK_CUR|SEEK_END|SEEK_"
    "SET|stdin|stdout|stderr)"};
const QRegularExpression kKeywordRegex{
    "\\b(alignas|alignof|asm|auto|bool|break|case|catch|char|char8_t|char16_t|char32_t|class|"
    "compl|concept|const|consteval|constexpr|constinit|const_cast|continue|co_await|co_return|co_"
    "yield|decltype|default|delete|do|double|dynamic_cast|else|enum|explicit|export|extern|false|"
    "final|float|for|friend|goto|if|import|inline|int|int8_t|int16_t|int32_t|int64_t|uint8_t|"
    "uint16_t|uint32_t|uint64_t|long|mutable|namespace|new|noexcept|nullptr|operator|override|"
    "private|protected|public|register|reinterpret_cast|requires|return|short|signed|sizeof|static|"
    "static_|assert|static_cast|struct|switch|template|this|thread_local|throw|true|try|typedef|"
    "typeid|typename|union|unsigned|using|virtual|void|volatile|wchar_t|while)\\b"};
const QRegularExpression kCapitalizedRegex{"(?<=[\\s\\(<])([A-Z][\\w]*)"};
// int Function( or Namespace::FunctionName( patterns
const QRegularExpression kFunctionDefinitionRegex{
    "(?<=\\w)\\s+(([A-Za-z_]\\w*::)*[A-Za-z_]\\w*)(?=\\()", PatternOption::CaseInsensitiveOption};
const QRegularExpression kOnlyUppercaseRegex{"(?<=[\\s\\(])([A-Z][0-9A-Z\\_]*)(?=\\b)"};
const QRegularExpression kCommaRegex{"([\\;\\,])"};
// Methods and variables from a namespace, after "::" (e.g. std::cout, absl::Milliseconds)
const QRegularExpression kNamespaceVariablesRegex{"(?<=::)([A-Za-z_]\\w*)(?=\\b)"};
// Namespaces itself, before "::"
const QRegularExpression kNamespaceRegex{"([A-Za-z_]\\w*::)"};
const QRegularExpression kClassNameRegex{
    "\\b((?:class|concept|enum|namespace|struct|typename)\\s+(\\w+))"};
// Variables starting with lowercase and finishing with _ (e.g. tracks_) or starting with "m_"
const QRegularExpression kClassMemberRegex{"\\b([a-z]\\w*\\_(?<=\\b)|(?<=\\b)m_\\w*)"};
const QRegularExpression kPreprocessorRegex{"((^\\s*)#\\s*[A-Za-z_]\\w*)"};
// Match with <word> after #include
const QRegularExpression kIncludeFileRegex{"((?<=#include)\\s*<[^>]*>)"};
const QRegularExpression kCommentRegex{
    "(\\/\\/(?:[^\\r\\n\\\\]|\\\\(?:\\r\\n?|\\n|(?![\\r\\n])))*|\\/\\*[\\s\\S]*?\\*\\/)"};
// Match with "/*" comments which starts but not finishes in this line
const QRegularExpression kOpenCommentRegex{"(\\/\\*([^\\*]|[\\*]+[^\\/])*?)$"};
// Match with the closing part of the comment
const QRegularExpression kEndCommentRegex{"([\\s\\S]*\\*\\/)"};
// Match with a line without a closing comment
const QRegularExpression kNoEndCommentRegex{"(([^\\*]|\\*+[^\\/\\*])*)$"};
// Similar process to comments to match a multiline string
const QRegularExpression kStringRegex{"(\"([^\\\\\"]|\\\\.)*\"|\'[^\']*\')"};
const QRegularExpression kOpenStringRegex{"(\"([^\\\\\"]|\\\\.)*\\\\)$"};
const QRegularExpression kEndStringRegex{"(([^\\\\\"]|\\\\.)*\")"};
const QRegularExpression kNoEndStringRegex{"(([^\\\\\"]|\\\\.)*\\\\)$"};
}  // namespace CppRegex
}  // namespace

Cpp::Cpp() : QSyntaxHighlighter{static_cast<QObject*>(nullptr)} {}

CppHighlighterState HighlightBlockCpp(
    const QString& code, int previous_block_state,
    std::function<void(int, int, const QTextCharFormat&)> set_format) {
  CppHighlighterState next_block_state = CppHighlighterState::kInitialState;
  const auto apply = [&code, &set_format, &next_block_state](
                         const QRegularExpression& expression, const QColor& color,
                         CppHighlighterState new_state = CppHighlighterState::kInitialState) {
    QTextCharFormat format{};
    format.setForeground(color);

    for (auto it = expression.globalMatch(code); it.hasNext();) {
      const auto match = it.next();
      // We use the first / outermost capture group, as this gives more flexibility for the match
      // without being highlighted. In particular this allows variable length matches before the
      // part of interest (in contrast to fixed length lookaheads).
      set_format(match.capturedStart(1), match.capturedLength(1), format);
      next_block_state = new_state;
    }
  };

  // We are processing line by line and trying to find all substrings that match with these
  // patterns. As each one paints over the others, order matters.

  // Ordered heuristics for painting certain word patterns. Should be at the first.
  apply(CppRegex::kCapitalizedRegex, kGreyViolet);
  apply(CppRegex::kNamespaceVariablesRegex, kGreyViolet);
  apply(CppRegex::kFunctionDefinitionRegex, kYellowOrange);
  apply(CppRegex::kNamespaceRegex, kGreyViolet);
  apply(CppRegex::kOnlyUppercaseRegex, kOlive);

  // Extra patterns which make the syntax highlighter nicer. Order doesn't matter.
  apply(CppRegex::kClassNameRegex, kGreyViolet);
  apply(CppRegex::kNumberRegex, kBlue);
  apply(CppRegex::kClassMemberRegex, kViolet);
  apply(CppRegex::kCommaRegex, kOrange);

  // Special C/C++ patterns. Order doesn't matter.
  apply(CppRegex::kKeywordRegex, kOrange);
  apply(CppRegex::kConstantRegex, kOlive);
  apply(CppRegex::kIncludeFileRegex, kGreen);
  apply(CppRegex::kPreprocessorRegex, kYellow);

  // Comments and strings should be painted at the end.
  apply(CppRegex::kStringRegex, kGreen);
  apply(CppRegex::kCommentRegex, kGrey);

  // For several-lines comments and strings, we have these states.
  if (previous_block_state == CppHighlighterState::kOpenStringState) {
    apply(CppRegex::kNoEndStringRegex, kGreen, CppHighlighterState::kOpenStringState);
    apply(CppRegex::kEndStringRegex, kGreen);
  }
  if (previous_block_state == CppHighlighterState::kOpenCommentState) {
    apply(CppRegex::kNoEndCommentRegex, kGrey, CppHighlighterState::kOpenCommentState);
    apply(CppRegex::kEndCommentRegex, kGrey);
  }
  apply(CppRegex::kOpenStringRegex, kGreen, CppHighlighterState::kOpenStringState);
  apply(CppRegex::kOpenCommentRegex, kGrey, CppHighlighterState::kOpenCommentState);

  return next_block_state;
}

void Cpp::highlightBlock(const QString& code) {
  setCurrentBlockState(HighlightBlockCpp(
      code, previousBlockState(), [this](int start, int count, const QTextCharFormat& format) {
        setFormat(start, count, format);
      }));
}
}  // namespace orbit_syntax_highlighter
