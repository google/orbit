// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbiteventiterator.h"

#include "ui_orbiteventiterator.h"

//-----------------------------------------------------------------------------
OrbitEventIterator::OrbitEventIterator(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitEventIterator) {
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
void OrbitEventIterator::SetFunctionName(const std::string& function_name) {
  ui->Label->setText(QString::fromStdString(function_name));
}
