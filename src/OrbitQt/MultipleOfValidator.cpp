// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/MultipleOfValidator.h"

namespace orbit_qt {

QValidator::State MultipleOfValidator::validate(QString& input, int& pos) const {
  if (input.isEmpty()) {
    return QValidator::State::Intermediate;
  }

  QValidator::State state = QIntValidator::validate(input, pos);
  if (state == QValidator::Invalid) {
    return state;
  }

  bool valid = false;
  int input_value = input.toInt(&valid);
  if (!valid) {
    return QValidator::State::Invalid;
  }
  if (input_value >= bottom() && input_value <= top() && input_value % multiple_of_ == 0) {
    return QValidator::State::Acceptable;
  }

  if (input_value <= top() && input.size() < QString::number(top()).size()) {
    return QValidator::State::Intermediate;
  }
  return QValidator::State::Invalid;
}
}  // namespace orbit_qt