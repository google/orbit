
// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ElidedLabel.h"

#include <QTextLayout>

void ElidedLabel::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QFontMetrics metrics = painter.fontMetrics();

  const QString content = text_;
  QTextLayout textLayout(content, painter.font());

  textLayout.beginLayout();
  QString elided_text =
      metrics.elidedText(content, elision_mode_, width() - 10);
  painter.drawText(QPoint(0, metrics.ascent()), elided_text);
  textLayout.endLayout();
}

void ElidedLabel::setTextWithElision(const QString& text,
                                     Qt::TextElideMode mode) {
  text_ = text;
  elision_mode_ = mode;
}