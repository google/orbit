// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/ElidedLabel.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QTextLayout>

void ElidedLabel::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QFontMetrics metrics = painter.fontMetrics();

  const QString content = text_;
  QTextLayout text_layout(content, painter.font());

  text_layout.beginLayout();
  QString elided_text = metrics.elidedText(content, elision_mode_, width() - 10);
  painter.drawText(QPoint(0, metrics.ascent()), elided_text);
  text_layout.endLayout();
  QLabel::paintEvent(event);
}

void ElidedLabel::setTextWithElision(const QString& text, Qt::TextElideMode mode) {
  text_ = text;
  elision_mode_ = mode;
}