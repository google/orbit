// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNTAX_HIGHLIGHTER_CPP_H_
#define SYNTAX_HIGHLIGHTER_CPP_H_

#include <QRegularExpression>
#include <QSyntaxHighlighter>

namespace orbit_syntax_highlighter {

//  This a syntax highlighter for C++.
//  It derives from QSyntaxHighlighter, so check out QSyntaxHighlighter's
//  documentation on how to use it. There are no additional settings or
//  APIs.

class Cpp : public QSyntaxHighlighter {
  Q_OBJECT
  void highlightBlock(const QString& code) override;

 public:
  explicit Cpp();

  static QRegularExpression comment_regex_;
  static QRegularExpression open_comment_regex_;
  static QRegularExpression end_comment_regex_;
  static QRegularExpression no_end_comment_regex_;
  static QRegularExpression function_definition_regex_;
  static QRegularExpression number_regex_;
  static QRegularExpression constant_regex_;
  static QRegularExpression keyword_regex_;
  static QRegularExpression preprocessor_regex_;
  static QRegularExpression include_file_regex_;
  static QRegularExpression string_regex_;
  static QRegularExpression open_string_regex_;
  static QRegularExpression end_string_regex_;
  static QRegularExpression no_end_string_regex_;
  static QRegularExpression comma_regex_;
  static QRegularExpression only_uppercase_regex_;
  static QRegularExpression capitalized_regex_;
  static QRegularExpression namespace_regex_;
  static QRegularExpression namespace_variables_regex_;
  static QRegularExpression class_name_regex_;
  static QRegularExpression class_member_regex_;
};

}  // namespace orbit_syntax_highlighter

#endif  // SYNTAX_HIGHLIGHTER_CPP_H_
