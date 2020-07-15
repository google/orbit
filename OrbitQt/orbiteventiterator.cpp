// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbiteventiterator.h"

#include "ui_orbiteventiterator.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
OrbitEventIterator::OrbitEventIterator(QWidget* parent)
    : QFrame(parent), ui(new Ui::OrbitEventIterator) {
  ui->setupUi(this);
}

//-----------------------------------------------------------------------------
OrbitEventIterator::~OrbitEventIterator() { delete ui; }

//-----------------------------------------------------------------------------
void OrbitEventIterator::on_NextButton_clicked() {
  if (next_button_callback_) {
    next_button_callback_();
  }
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::on_PreviousButton_clicked() {
  if (previous_button_callback_) {
    previous_button_callback_();
  }
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::on_DeleteButton_clicked() {
  if (delete_button_callback_) {
    delete_button_callback_();
  }
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::SetFunctionName(const std::string& function_name) {
  ui->Label->setText(QString::fromStdString(function_name));
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::SetMaxCount(int max_count) {
  max_count_ = max_count;
  UpdateCountLabel();
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::SetIndex(int current_index) {
  current_index_ = 0;
  UpdateCountLabel();
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::IncrementIndex() {
  if (current_index_ < max_count_ - 1) {
    ++current_index_;
    UpdateCountLabel();
  }
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::DecrementIndex() {
  if (current_index_ > 0) {
    --current_index_;
    UpdateCountLabel();
  }
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::UpdateCountLabel() {
  ui->CountLabel->setText(QString::fromStdString(
    absl::StrFormat("%d / %d", current_index_, max_count_)));
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::HideDeleteButton() {
  ui->DeleteButton->hide();
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::EnableButtons() {
  ui->NextButton->setEnabled(true);
  ui->PreviousButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
void OrbitEventIterator::DisableButtons() {
  ui->NextButton->setEnabled(false);
  ui->PreviousButton->setEnabled(false);
}