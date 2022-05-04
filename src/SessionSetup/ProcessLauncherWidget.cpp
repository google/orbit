// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ProcessLauncherWidget.h"

#include <QFileDialog>
#include <QLineEdit>

#include "OrbitBase/Logging.h"
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

void ProcessLauncherWidget::on_BrowseProcessButton_clicked() {
  QString file = QFileDialog::getOpenFileName(this, "Select an executable file...");
  if (!file.isEmpty()) {
    ui_->ProcessComboBox->lineEdit()->setText(file);
  }
}

void ProcessLauncherWidget::on_LaunchButton_clicked() {
  // TODO(b/225906734): Windows Process Launcher
  // The code below is for reference. It shows how a subsequent PR will trigger a process launch and
  // set the error label on failure.
  //
  // ORBIT_CHECK(process_manager_ != nullptr);
  // QString process = ui->ProcessComboBox->lineEdit()->text();
  // QString working_dir = ui->WorkingDirComboBox->lineEdit()->text();
  // QString args = ui->ArgumentsComboBox->lineEdit()->text();
  // bool pause_on_entry_point = ui->PauseAtEntryPoingCheckBox->isChecked();
  //
  // orbit_grpc_protos::ProcessToLaunch process_to_launch;
  // process_to_launch.set_executable_path(process.toStdString());
  // process_to_launch.set_working_directory(working_dir.toStdString());
  // process_to_launch.set_arguments(args.toStdString());
  // process_to_launch.set_spin_at_entry_point(pause_on_entry_point);
  // auto result = process_manager_->LaunchProcess(process_to_launch);
  //  if (result.has_error()) {
  //    ui->ErrorLabel->setText(result.error().message().c_str());
  // }
  // else {
  //  emit ProcessLaunched(result.value());
  // }
}

void ProcessLauncherWidget::on_BrowseWorkingDirButton_clicked() {
  QString directory = QFileDialog::getExistingDirectory(this, "Select a working directory");
  if (!directory.isEmpty()) {
    ui_->WorkingDirComboBox->lineEdit()->setText(directory);
  }
}

void ProcessLauncherWidget::SetProcessManager(
    orbit_client_services::ProcessManager* process_manager) {
  process_manager_ = process_manager;
}

}  // namespace orbit_session_setup