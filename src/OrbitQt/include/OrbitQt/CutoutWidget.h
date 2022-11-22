// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CUTOUT_WIDGET_
#define ORBIT_QT_CUTOUT_WIDGET_

#include <QLabel>

/**
Used by the TutorialOverlay widget (TutorialOverlay.h).

This widget does not provide any functionality on top of QLabel, so there is no
reason to use it except in cases explicitly stated in the documentation of
TutorialOverlay.

CutoutWidget merely provides a semantic difference when placed inside the TutorialOverlay
widget, as TutorialOverlay is looking for instances of this class to determine
the "area of interest" in each tutorial step.
**/
class CutoutWidget : public QLabel {
  Q_OBJECT

 public:
  explicit CutoutWidget(QWidget* parent = nullptr) : QLabel{parent} {
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
  }
};

#endif
