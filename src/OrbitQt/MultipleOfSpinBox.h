// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef ORBIT_QT_Q_MULTIPLE_OF_EIGHT_SPIN_BOX_H_
#define ORBIT_QT_Q_MULTIPLE_OF_EIGHT_SPIN_BOX_H_

#include <QSpinBox>

namespace orbit_qt {

// A specialization of `QIntValidator` that only accepts values that are a multiple of
// `multiple_of_`.
class MultipleOfValidator : public QIntValidator {
 public:
  explicit MultipleOfValidator(QObject* parent = nullptr) : QIntValidator(parent) {}
  MultipleOfValidator(int bottom, int top, int multiple_of, QObject* parent = nullptr)
      : QIntValidator(bottom, top, parent), multiple_of_{multiple_of} {}
  void setMultipleOf(int multiple_of) { multiple_of_ = multiple_of; }
  [[nodiscard]] QValidator::State validate(QString& input, int& pos) const override;

 private:
  int multiple_of_ = 1;
};

// A `QSpinBox` that takes a custom validator. If the `validator_` is not set (nullptr), it will
// fallback to `QSpinBox::validate`.
class MultipleOfSpinBox : public QSpinBox {
 public:
  explicit MultipleOfSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {}
  void SetValidator(std::unique_ptr<QIntValidator> validator) { validator_ = std::move(validator); }

 protected:
  QValidator::State validate(QString& input, int& pos) const override;

 private:
  std::unique_ptr<QIntValidator> validator_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_Q_MULTIPLE_OF_EIGHT_SPIN_BOX_H_
