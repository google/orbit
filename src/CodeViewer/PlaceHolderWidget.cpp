// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeViewer/PlaceHolderWidget.h"

#include <QPaintEvent>
#include <QWheelEvent>

namespace orbit_code_viewer {

void PlaceHolderWidget::paintEvent(QPaintEvent* event) { emit PaintEventTriggered(event); }

void PlaceHolderWidget::wheelEvent(QWheelEvent* event) { emit WheelEventTriggered(event); }

}  // namespace orbit_code_viewer