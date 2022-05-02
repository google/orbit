// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef ORBIT_QT_Q_MULTIPLE_OF_EIGHT_SPIN_BOX_H_
#define ORBIT_QT_Q_MULTIPLE_OF_EIGHT_SPIN_BOX_H_

#include <QSpinBox>

namespace orbit_qt {

// A specialization of QSpinBox that only accepts values that are a multiple of the `singleStep`
// size. E.g. a `MultipleOfSpinBox` with a single step size of 8, only accepts values that are
// a multiple of 8.
class MultipleOfSpinBox : public QSpinBox {
 public:
  MultipleOfSpinBox(QWidget* parent) : QSpinBox(parent) {}
  QValidator::State validate(QString& input, int& pos) const override;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_Q_MULTIPLE_OF_EIGHT_SPIN_BOX_H_
