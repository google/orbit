// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/MultipleOfSpinBox.h"

#include "OrbitQt/MultipleOfValidator.h"

namespace orbit_qt {

QValidator::State MultipleOfSpinBox::validate(QString& input, int& pos) const {
  MultipleOfValidator validator{minimum(), maximum(), singleStep()};
  return validator.validate(input, pos);
}

}  // namespace orbit_qt
