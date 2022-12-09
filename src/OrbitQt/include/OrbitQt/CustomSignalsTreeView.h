// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CUSTOM_SIGNALS_TREE_VIEW_H_
#define ORBIT_QT_CUSTOM_SIGNALS_TREE_VIEW_H_

#include <QKeyEvent>
#include <QMouseEvent>
#include <QTreeView>

class CustomSignalsTreeView : public QTreeView {
  Q_OBJECT

 public:
  explicit CustomSignalsTreeView(QWidget* parent = nullptr) : QTreeView(parent) {}

  void keyPressEvent(QKeyEvent* event) override {
    if (event->matches(QKeySequence::Copy)) {
      emit copyKeySequencePressed();
    } else {
      QTreeView::keyPressEvent(event);
    }
  }

  void mousePressEvent(QMouseEvent* event) override {
    QModelIndex index = indexAt(event->pos());
    bool alt_pressed = (event->modifiers() & Qt::AltModifier) != 0u;
    if (index.isValid() && alt_pressed) {
      emit altKeyAndMousePressed(event->pos());
    } else {
      QTreeView::mousePressEvent(event);
    }
  }

 signals:
  void copyKeySequencePressed();
  void altKeyAndMousePressed(const QPoint& point);
};

#endif  // ORBIT_QT_CUSTOM_SIGNALS_TREE_VIEW_H_
