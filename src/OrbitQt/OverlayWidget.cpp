// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OverlayWidget.h"

#include <QColor>
#include <QPainter>

#include "OrbitBase/Logging.h"

namespace {

// This color is used as a "background" for the overlay. The alpha value makes it transparent
const QColor kOverlayShadeColor{100, 100, 100, 200};

}  // namespace

namespace orbit_qt {

OverlayWidget::OverlayWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::OverlayWidget>()) {
  CHECK(parent != nullptr);
  parent->installEventFilter(this);
  ui_->setupUi(this);
  ui_->cancelButton->setEnabled(true);

  QObject::connect(ui_->cancelButton, &QPushButton::clicked, this, [this] { emit Cancelled(); });
}

void OverlayWidget::paintEvent(QPaintEvent* /*event*/) {
  QPainter(this).fillRect(rect(), kOverlayShadeColor);
}

bool OverlayWidget::eventFilter(QObject* obj, QEvent* event) {
  if (obj == parent() && event->type() == QEvent::Resize) {
    resize(parentWidget()->size());
  }
  return false;
}

}  // namespace orbit_qt