// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_EVENT_ITERATOR_H_
#define ORBIT_EVENT_ITERATOR_H_

#include <QWidget>

#include "types.h"

namespace Ui {
class OrbitEventIterator;
}

class OrbitEventIterator : public QWidget {
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

  void SetFunctionName(const std::string& function);

 private slots:
  void on_NextButton_clicked();
  void on_PreviousButton_clicked();

 protected:
  Ui::OrbitEventIterator* ui;
  std::function<void(void)> next_button_callback_;
  std::function<void(void)> previous_button_callback_;
};

#endif  // ORBIT_EVENT_ITERATOR_H_