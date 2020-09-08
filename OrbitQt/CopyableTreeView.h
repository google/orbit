// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_COPYABLE_TREE_VIEW_H_
#define ORBIT_QT_COPYABLE_TREE_VIEW_H_

#include <QKeyEvent>
#include <QTreeView>

class CopyableTreeView : public QTreeView {
  Q_OBJECT

 public:
  explicit CopyableTreeView(QWidget* parent = nullptr) : QTreeView(parent) {}

  void keyPressEvent(QKeyEvent* event) override {
    if (event->matches(QKeySequence::Copy)) {
      emit copyKeySequencePressed();
    } else {
      QTreeView::keyPressEvent(event);
    }
  }

 signals:
  void copyKeySequencePressed();
};

#endif  // ORBIT_QT_COPYABLE_TREE_VIEW_H_
