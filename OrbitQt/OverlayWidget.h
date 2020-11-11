// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_OVERLAY_WIDGET_H_
#define ORBIT_QT_OVERLAY_WIDGET_H_

#include <QEvent>
#include <QObject>
#include <QPainter>
#include <QWidget>
#include <memory>

#include "OrbitBase/Logging.h"
#include "ui_OverlayWidget.h"

namespace OrbitQt {

class OverlayWidget : public QWidget {
  Q_OBJECT
 public:
  explicit OverlayWidget(QWidget* parent);

  void Activate(const QString& message, const std::function<void()>& cancel_callback = nullptr,
                const QString& cancel_button_text = "Cancel");
  void Deactivate();
  void UpdateCancelButton(const std::function<void()>& cancel_callback,
                          const QString& cancel_button_text = "Cancel");
  void UpdateMessage(const QString& message) { ui_->messageLabel->setText(message); }
  void StartSpinner() { ui_->progressBar->setVisible(true); }
  void StopSpinner() { ui_->progressBar->setVisible(false); }

 protected:
  void paintEvent(QPaintEvent* /*event*/) override;
  bool eventFilter(QObject* obj, QEvent* event) override;

 private slots:
  void on_cancelButton_clicked();

 private:
  QWidget* parent_;
  std::unique_ptr<Ui::OverlayWidget> ui_;
  std::function<void()> cancel_callback_ = nullptr;
};

}  // namespace OrbitQt

#endif  // ORBIT_QT_OVERLAY_WIDGET_H_