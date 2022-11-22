// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNTAX_HIGHLIGHTER_CPP_H_
#define SYNTAX_HIGHLIGHTER_CPP_H_

#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <functional>

namespace orbit_syntax_highlighter {

//  This a syntax highlighter for C++.
//  It derives from QSyntaxHighlighter, so check out QSyntaxHighlighter's
//  documentation on how to use it. There are no additional settings or
//  APIs.

enum CppHighlighterState { kInitialState, kOpenCommentState, kOpenStringState };

class Cpp : public QSyntaxHighlighter {
  Q_OBJECT
  void highlightBlock(const QString& code) override;

 public:
  explicit Cpp();
};

CppHighlighterState HighlightBlockCpp(
    const QString& code, int previous_block_state,
    std::function<void(int, int, const QTextCharFormat&)> set_format);
}  // namespace orbit_syntax_highlighter

#endif  // SYNTAX_HIGHLIGHTER_CPP_H_
