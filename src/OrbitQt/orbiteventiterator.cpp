// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/orbiteventiterator.h"

#include <absl/strings/str_format.h>

#include <QLabel>
#include <QPushButton>
#include <algorithm>

#include "OrbitQt/ElidedLabel.h"
#include "ui_orbiteventiterator.h"

OrbitEventIterator::OrbitEventIterator(QWidget* parent)
    : QFrame(parent), ui(new Ui::OrbitEventIterator) {
  ui->setupUi(this);
}

OrbitEventIterator::~OrbitEventIterator() { delete ui; }

void OrbitEventIterator::on_NextButton_clicked() {
  if (next_button_callback_) {
    next_button_callback_();
  }
}

void OrbitEventIterator::on_PreviousButton_clicked() {
  if (previous_button_callback_) {
    previous_button_callback_();
  }
}

void OrbitEventIterator::on_DeleteButton_clicked() {
  if (delete_button_callback_) {
    delete_button_callback_();
  }
}

void OrbitEventIterator::SetFunctionName(std::string_view function_name) {
  ui->Label->setTextWithElision(QString::fromUtf8(function_name.data(), function_name.size()));
}

void OrbitEventIterator::SetMinMaxTime(uint64_t min_time, uint64_t max_time) {
  min_time_ = min_time;
  max_time_ = max_time;
  current_time_ = std::min(current_time_, max_time_);
  current_time_ = std::max(current_time_, min_time_);
  UpdatePositionLabel();
}
void OrbitEventIterator::SetCurrentTime(uint64_t current_time) {
  current_time_ = current_time;
  UpdatePositionLabel();
}

void OrbitEventIterator::UpdatePositionLabel() {
  double fraction =
      static_cast<double>(current_time_ - min_time_) / static_cast<double>(max_time_ - min_time_);
  ui->position_label_->setText(QString::fromStdString(absl::StrFormat("%.6f", fraction)));
}

void OrbitEventIterator::HideDeleteButton() { ui->DeleteButton->hide(); }

void OrbitEventIterator::EnableButtons() {
  ui->NextButton->setEnabled(true);
  ui->PreviousButton->setEnabled(true);
}

void OrbitEventIterator::DisableButtons() {
  ui->NextButton->setEnabled(false);
  ui->PreviousButton->setEnabled(false);
}
