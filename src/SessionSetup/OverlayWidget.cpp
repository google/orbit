// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/OverlayWidget.h"

#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>

#include "OrbitBase/Logging.h"
#include "ui_OverlayWidget.h"

namespace {

// This color is used as a "background" for the overlay. The alpha value makes it transparent
const QColor kOverlayShadeColor{100, 100, 100, 200};

}  // namespace

namespace orbit_session_setup {

// The destructor needs to be defined here because it needs to see the type
// `Ui::LoadCaptureWidget`. The header file only contains a forward declaration.
OverlayWidget::~OverlayWidget() = default;

OverlayWidget::OverlayWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::OverlayWidget>()) {
  ORBIT_CHECK(parent != nullptr);
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

void OverlayWidget::SetSpinning(bool value) { ui_->progressBar->setVisible(value); }

void OverlayWidget::SetCancelable(bool value) { ui_->cancelButton->setVisible(value); }

void OverlayWidget::SetStatusMessage(const QString& message) {
  ui_->messageLabel->setText(message);
}

void OverlayWidget::SetButtonMessage(const QString& message) {
  ui_->cancelButton->setText(message);
}

bool OverlayWidget::IsSpinning() const { return ui_->progressBar->isVisible(); }

bool OverlayWidget::IsCancelable() const { return ui_->cancelButton->isVisible(); }

QString OverlayWidget::GetStatusMessage() const { return ui_->messageLabel->text(); }

QString OverlayWidget::GetButtonMessage() const { return ui_->cancelButton->text(); }

}  // namespace orbit_session_setup