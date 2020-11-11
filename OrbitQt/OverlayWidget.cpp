// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OverlayWidget.h"

#include <QMovie>

namespace OrbitQt {

OverlayWidget::OverlayWidget(QWidget* parent)
    : QWidget(parent), parent_(parent), ui_(std::make_unique<Ui::OverlayWidget>()) {
  CHECK(parent != nullptr);
  parent->installEventFilter(this);
  ui_->setupUi(this);
  ui_->cancelButton->setEnabled(true);
}

void OverlayWidget::Activate(const QString& message, const std::function<void()>& cancel_callback,
                             const QString& cancel_button_text) {
  UpdateMessage(message);
  ui_->cancelButton->setText(cancel_button_text);
  setVisible(true);
  StartSpinner();
  ui_->cancelButton->setVisible(cancel_callback != nullptr);
  if (cancel_callback != nullptr) {
    cancel_callback_ = cancel_callback;
  }
}

void OverlayWidget::Deactivate() {
  setVisible(false);
  cancel_callback_ = nullptr;
}

void OverlayWidget::UpdateCancelButton(const std::function<void()>& cancel_callback,
                                       const QString& cancel_button_text) {
  CHECK(cancel_callback != nullptr);
  CHECK(cancel_callback_ != nullptr);

  cancel_callback_ = cancel_callback;
  ui_->cancelButton->setText(cancel_button_text);
}

void OverlayWidget::paintEvent(QPaintEvent* /*event*/) {
  QPainter(this).fillRect(rect(), {100, 100, 100, 128});
}

bool OverlayWidget::eventFilter(QObject* obj, QEvent* event) {
  if (!obj->isWidgetType()) return false;

  auto* widget = dynamic_cast<QWidget*>(obj);
  CHECK(widget == parent_);

  if (event->type() == QEvent::Resize) {
    resize(parent_->size());
  }
  return false;
}

void OverlayWidget::on_cancelButton_clicked() {
  CHECK(cancel_callback_ != nullptr);
  cancel_callback_();
  Deactivate();
}

}  // namespace OrbitQt