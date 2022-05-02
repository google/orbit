// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MultipleOfSpinBox.h"

namespace orbit_qt {

QValidator::State MultipleOfSpinBox::validate(QString& input, int& pos) const {
  if (input.isEmpty()) {
    return QValidator::State::Intermediate;
  }

  QIntValidator validator;
  validator.setBottom(minimum());
  validator.setTop(maximum());
  QValidator::State state = validator.validate(input, pos);
  if (state == QValidator::Invalid) {
    return state;
  }

  bool valid = false;
  int input_value = input.toInt(&valid);
  if (!valid) {
    return QValidator::State::Invalid;
  }
  if (input_value >= minimum() && input_value <= maximum() && input_value % singleStep() == 0) {
    return QValidator::State::Acceptable;
  }
  if (input_value <= maximum()) {
    return QValidator::State::Intermediate;
  }
  return QValidator::State::Invalid;
}
}  // namespace orbit_qt
