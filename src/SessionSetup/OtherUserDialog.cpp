// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/OtherUserDialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QSettings>
#include <memory>
#include <string>

#include "ui_OtherUserDialog.h"

constexpr const char* kRememberKey = "OtherUserDialog.RememberKey";

namespace orbit_session_setup {

OtherUserDialog::OtherUserDialog(const QString& user_name, QWidget* parent)
    : QDialog(parent), ui_(std::make_unique<Ui::OtherUserDialog>()) {
  ui_->setupUi(this);
  ui_->userLabel->setText(user_name);
}

OtherUserDialog::~OtherUserDialog() = default;

ErrorMessageOr<void> OtherUserDialog::Exec() {
  QSettings settings;
  if (settings.contains(kRememberKey)) {
    return outcome::success();
  }

  int rc = exec();

  if (rc != QDialog::Accepted) {
    return ErrorMessage{"user rejected"};
  }

  if (ui_->rememberCheckbox->isChecked()) {
    settings.setValue(kRememberKey, kRememberKey);
  }

  return outcome::success();
}

}  // namespace orbit_session_setup