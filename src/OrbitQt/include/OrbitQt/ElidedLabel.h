// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ELIDED_LABEL_H_
#define ORBIT_QT_ELIDED_LABEL_H_

#include <QLabel>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QString>
#include <QStyle>
#include <QWidget>
#include <Qt>

class ElidedLabel : public QLabel {
  Q_OBJECT

 public:
  explicit ElidedLabel(QWidget* parent) : QLabel(parent) {}
  void setTextWithElision(const QString& text, Qt::TextElideMode mode = Qt::ElideMiddle);

 protected:
  void paintEvent(QPaintEvent* event) override;

  QString text_;
  Qt::TextElideMode elision_mode_;
};

#endif  // ORBIT_QT_ELIDED_LABEL_H_