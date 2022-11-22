// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNTAX_HIGHLIGHTER_X86_ASSEMBLY_H_
#define SYNTAX_HIGHLIGHTER_X86_ASSEMBLY_H_

#include <absl/container/flat_hash_map.h>

#include <QColor>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <Qt>
#include <functional>

namespace orbit_syntax_highlighter {

/*
  This a syntax highlighter for x86 and x86_64 assembly (Intel syntax).

  It derives from QSyntaxHighlighter, so check out QSyntaxHighlighter's
  documentation on how to use it. There are no additional settings or
  APIs.
*/
class X86Assembly : public QSyntaxHighlighter {
  Q_OBJECT

  void highlightBlock(const QString& code) override;

 public:
  explicit X86Assembly();
};

void HighlightBlockAssembly(
    const QString& code, const std::function<void(int, int, const QTextCharFormat&)>& set_format);
void HighlightAnnotatingBlock(
    const QString& code, const std::function<void(int, int, const QTextCharFormat&)>& set_format,
    const QColor& default_color = Qt::white);

}  // namespace orbit_syntax_highlighter

#endif  // SYNTAX_HIGHLIGHTER_X86_ASSEMBLY_H_