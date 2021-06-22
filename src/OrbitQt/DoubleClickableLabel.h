// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_DOUBLE_CLICKABLE_LABEL_H_
#define ORBIT_QT_DOUBLE_CLICKABLE_LABEL_H_

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>

namespace orbit_qt {

class DoubleClickableLabel : public QLabel {
  Q_OBJECT

 public:
  using QLabel::QLabel;

 signals:
  void DoubleClicked();

 protected:
  void mouseDoubleClickEvent(QMouseEvent* event) override {
    QWidget::mouseDoubleClickEvent(event);
    emit DoubleClicked();
  }
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_DOUBLE_CLICKABLE_LABEL_H_
