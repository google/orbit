// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ProcessLauncherWidget.h"

#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "ui_ProcessLauncherWidget.h"

namespace orbit_session_setup {

ProcessLauncherWidget::ProcessLauncherWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::ProcessLauncherWidget>()) {
  ui_->setupUi(this);
  ui_->ProcessComboBox->lineEdit()->setPlaceholderText("Process");
  ui_->WorkingDirComboBox->lineEdit()->setPlaceholderText("Working directory");
  ui_->ArgumentsComboBox->lineEdit()->setPlaceholderText("Arguments");
  ui_->ErrorLabel->setText("");
  ui_->gridLayout_2->setColumnStretch(0, 90);
}

ProcessLauncherWidget ::~ProcessLauncherWidget() {}

void ProcessLauncherWidget::SetProcessManager(
    orbit_client_services::ProcessManager* process_manager) {
  process_manager_ = process_manager;
}

void ProcessLauncherWidget::on_BrowseProcessButton_clicked() {
  QString file = QFileDialog::getOpenFileName(this, "Select an executable file...");
  if (!file.isEmpty()) {
    ui_->ProcessComboBox->lineEdit()->setText(file);
  }
}

void ProcessLauncherWidget::on_BrowseWorkingDirButton_clicked() {
  QString directory = QFileDialog::getExistingDirectory(this, "Select a working directory");
  if (!directory.isEmpty()) {
    ui_->WorkingDirComboBox->lineEdit()->setText(directory);
  }
}

void ProcessLauncherWidget::on_LaunchButton_clicked() {
  // TODO(b/225906734): Windows Process Launcher
}

}  // namespace orbit_session_setup