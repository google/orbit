// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_Q_MULTIPLE_OF_SPIN_BOX_H_
#define ORBIT_QT_Q_MULTIPLE_OF_SPIN_BOX_H_

#include <QSpinBox>
#include <QString>
#include <QValidator>
#include <QWidget>
#include <memory>

namespace orbit_qt {

// A `QSpinBox` that takes a custom validator. If the `validator_` is not set (nullptr), it will
// fallback to `QSpinBox::validate`.
class MultipleOfSpinBox : public QSpinBox {
 public:
  explicit MultipleOfSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {}

 protected:
  QValidator::State validate(QString& input, int& pos) const override;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_Q_MULTIPLE_OF_SPIN_BOX_H_
