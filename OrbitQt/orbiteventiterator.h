// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_EVENT_ITERATOR_H_
#define ORBIT_QT_ORBIT_EVENT_ITERATOR_H_

#include <QFrame>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QTextLayout>

#include "Profiling.h"
#include "absl/container/flat_hash_map.h"
#include "types.h"

namespace Ui {
class OrbitEventIterator;
}

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
  void SetMinMaxTime(TickType min_time_us, TickType max_time_us);
  void SetCurrentTime(TickType current_time_us);

  void DisableButtons();
  void EnableButtons();
  void HideDeleteButton();

 private slots:
  void on_NextButton_clicked();
  void on_PreviousButton_clicked();
  void on_DeleteButton_clicked();

 protected:
  void UpdatePositionLabel();
  Ui::OrbitEventIterator* ui;
  std::function<void(void)> next_button_callback_;
  std::function<void(void)> previous_button_callback_;
  std::function<void(void)> delete_button_callback_;

  TickType min_time_;
  TickType max_time_;
  TickType current_time_;
};

#endif  // ORBIT_QT_ORBIT_EVENT_ITERATOR_H_
