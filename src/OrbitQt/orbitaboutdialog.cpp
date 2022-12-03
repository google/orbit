// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/orbitaboutdialog.h"

#include <QLabel>
#include <QPlainTextEdit>

#include "ui_orbitaboutdialog.h"

namespace orbit_qt {

OrbitAboutDialog::OrbitAboutDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui::OrbitAboutDialog) {
  ui_->setupUi(this);
}

OrbitAboutDialog::~OrbitAboutDialog() = default;

void OrbitAboutDialog::SetLicenseText(const QString& text) {
  ui_->licenseTextEdit->setPlainText(text);
}

void OrbitAboutDialog::SetVersionString(const QString& version) {
  ui_->versionLabel->setText(QString{"Version %1"}.arg(version));
}

void OrbitAboutDialog::SetBuildInformation(const QString& build_info) {
  ui_->buildInformationEdit->setPlainText(build_info);
}

void OrbitAboutDialog::SetOpenGlRenderer(const QString& opengl_renderer, bool software_rendering) {
  auto label = QString{"OpenGL Renderer: %1"}.arg(opengl_renderer);
  if (software_rendering) {
    label.append(" (Software Rendering)");
  }

  ui_->opengl_renderer_label->setText(label);
}

}  // namespace orbit_qt
