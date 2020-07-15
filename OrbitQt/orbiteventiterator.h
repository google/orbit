// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_EVENT_ITERATOR_H_
#define ORBIT_EVENT_ITERATOR_H_

#include <QFrame>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QTextLayout>

#include "absl/container/flat_hash_map.h"
#include "types.h"

namespace Ui {
class OrbitEventIterator;
}

// TODO: Move into its own file(s).
class ElidedLabel : public QLabel {
  Q_OBJECT
 public:
  explicit ElidedLabel(QWidget* parent)
      : QLabel(parent) /*, metrics_(font())*/ {}
  void setText(const QString& text);

 protected:
  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    QFontMetrics metrics = painter.fontMetrics();

    QTextLayout textLayout(text_, painter.font());
    textLayout.beginLayout();
    QString elided_text = metrics.elidedText(text_, Qt::ElideMiddle, width());
    painter.drawText(QPoint(0, metrics.ascent()), elided_text);
    textLayout.endLayout();
  }
  QString text_;
};

class OrbitEventIterator : public QFrame {
  Q_OBJECT

 public:
  explicit OrbitEventIterator(QWidget* parent = nullptr);
  ~OrbitEventIterator() override;

  void SetNextButtonCallback(const std::function<void(void)>& callback) {
    next_button_callback_ = callback;
  }

  void SetPreviousButtonCallback(const std::function<void(void)>& callback) {
    previous_button_callback_ = callback;
  }

  void SetDeleteButtonCallback(const std::function<void(void)>& callback) {
    delete_button_callback_ = callback;
  }

  void SetFunctionName(const std::string& function);
  void SetMaxCount(int max_count);
  void SetIndex(int current_index);

  void IncrementIndex();
  void DecrementIndex();

  void DisableButtons();
  void EnableButtons();
  void HideDeleteButton();

 private slots:
  void on_NextButton_clicked();
  void on_PreviousButton_clicked();
  void on_DeleteButton_clicked();

 protected:
  void UpdateCountLabel();
  Ui::OrbitEventIterator* ui;
  std::function<void(void)> next_button_callback_;
  std::function<void(void)> previous_button_callback_;
  std::function<void(void)> delete_button_callback_;
  int max_count_ = 0;
  int current_index_ = 0;
};

#endif  // ORBIT_EVENT_ITERATOR_H_