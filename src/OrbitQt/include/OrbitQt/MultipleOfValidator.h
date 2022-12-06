// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_Q_MULTIPLE_OF_VALIDATOR_H_
#define ORBIT_QT_Q_MULTIPLE_OF_VALIDATOR_H_

#include <QIntValidator>
#include <QObject>
#include <QString>
#include <QValidator>

namespace orbit_qt {

// A specialization of `QIntValidator` that only accepts values that are a multiple of
// `multiple_of_`.
class MultipleOfValidator : public QIntValidator {
 public:
  explicit MultipleOfValidator(QObject* parent = nullptr) : QIntValidator(parent) {}
  MultipleOfValidator(int bottom, int top, int multiple_of, QObject* parent = nullptr)
      : QIntValidator(bottom, top, parent), multiple_of_{multiple_of} {}
  [[nodiscard]] QValidator::State validate(QString& input, int& pos) const override;

 private:
  int multiple_of_ = 1;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_Q_MULTIPLE_OF_VALIDATOR_H_
