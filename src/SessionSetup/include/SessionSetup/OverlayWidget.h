// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_OVERLAY_WIDGET_H_
#define SESSION_SETUP_OVERLAY_WIDGET_H_

#include <QEvent>
#include <QLabel>
#include <QObject>
#include <QPaintEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <memory>

namespace Ui {
class OverlayWidget;
}

namespace orbit_session_setup {

class OverlayWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool spinning READ IsSpinning WRITE SetSpinning)
  Q_PROPERTY(bool cancelable READ IsCancelable WRITE SetCancelable);
  Q_PROPERTY(QString statusMessage READ GetStatusMessage WRITE SetStatusMessage)
  Q_PROPERTY(QString buttonMessage READ GetButtonMessage WRITE SetButtonMessage);

 public:
  explicit OverlayWidget(QWidget* parent);
  ~OverlayWidget() override;

 protected:
  void paintEvent(QPaintEvent* /*event*/) override;
  bool eventFilter(QObject* obj, QEvent* event) override;

 public slots:
  void SetSpinning(bool value);
  void SetCancelable(bool value);
  void SetStatusMessage(const QString& message);
  void SetButtonMessage(const QString& message);

 signals:
  void Cancelled();

 private:
  std::unique_ptr<Ui::OverlayWidget> ui_;

  [[nodiscard]] bool IsSpinning() const;
  [[nodiscard]] bool IsCancelable() const;
  [[nodiscard]] QString GetStatusMessage() const;
  [[nodiscard]] QString GetButtonMessage() const;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_OVERLAY_WIDGET_H_