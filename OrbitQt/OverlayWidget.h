// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_OVERLAY_WIDGET_H_
#define ORBIT_QT_OVERLAY_WIDGET_H_

#include <QEvent>
#include <QLabel>
#include <QObject>
#include <QPaintEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <memory>

#include "ui_OverlayWidget.h"

namespace orbit_qt {

class OverlayWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool spinning READ IsSpinning WRITE SetSpinning)
  Q_PROPERTY(bool cancelable READ IsCancelable WRITE SetCancelable);
  Q_PROPERTY(QString statusMessage READ GetStatusMessage WRITE SetStatusMessage)
  Q_PROPERTY(QString buttonMessage READ GetButtonMessage WRITE SetButtonMessage);

 public:
  explicit OverlayWidget(QWidget* parent);

 protected:
  void paintEvent(QPaintEvent* /*event*/) override;
  bool eventFilter(QObject* obj, QEvent* event) override;

 public slots:
  void SetSpinning(bool value) { ui_->progressBar->setVisible(value); }
  void SetCancelable(bool value) { ui_->cancelButton->setVisible(value); }
  void SetStatusMessage(const QString& message) { ui_->messageLabel->setText(message); }
  void SetButtonMessage(const QString& message) { ui_->cancelButton->setText(message); }

 signals:
  void Cancelled();

 private:
  std::unique_ptr<Ui::OverlayWidget> ui_;

  [[nodiscard]] bool IsSpinning() const { return ui_->progressBar->isVisible(); }
  [[nodiscard]] bool IsCancelable() const { return ui_->cancelButton->isVisible(); }
  [[nodiscard]] QString GetStatusMessage() const { return ui_->messageLabel->text(); }
  [[nodiscard]] QString GetButtonMessage() const { return ui_->cancelButton->text(); }
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_OVERLAY_WIDGET_H_