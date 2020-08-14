// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitaboutdialog.h"

namespace OrbitQt {

OrbitAboutDialog::OrbitAboutDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui::OrbitAboutDialog) {
  ui_->setupUi(this);
}

void OrbitAboutDialog::SetLicenseText(const QString& text) {
  ui_->licenseTextEdit->setPlainText(text);
}

void OrbitAboutDialog::SetVersionString(const QString& version) {
  ui_->versionLabel->setText(QString{"Version %1"}.arg(version));
}

void OrbitAboutDialog::SetBuildInformation(const QString& build_info) {
  ui_->buildInformationEdit->setPlainText(build_info);
}

}  // namespace OrbitQt
