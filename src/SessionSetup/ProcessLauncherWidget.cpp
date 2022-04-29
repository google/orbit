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
    : QWidget(parent),
      ui(new Ui::ProcessLauncherWidget),
      last_launched_process_or_error_(ErrorMessage()) {
  ui->setupUi(this);
  ui->ProcessComboBox->lineEdit()->setPlaceholderText("Process");
  ui->WorkingDirComboBox->lineEdit()->setPlaceholderText("Working directory");
  ui->ArgumentsComboBox->lineEdit()->setPlaceholderText("Arguments");
  ui->ErrorLabel->setText("");

  ui->gridLayout_2->setColumnStretch(0, 90);
}

ProcessLauncherWidget::~ProcessLauncherWidget() { delete ui; }

void ProcessLauncherWidget::on_BrowseProcessButton_clicked() {
  QStringList list = QFileDialog::getOpenFileNames(this, "Select an executable file...");
  for (auto& file : list) {
    ui->ProcessComboBox->lineEdit()->setText(file);
    break;
  }
}

void ProcessLauncherWidget::on_LaunchButton_clicked() {
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
  // last_launched_process_or_error_ = process_manager_->LaunchProcess(process_to_launch);
  //  if (last_launched_process_or_error_.has_error()) {
  //    ui->ErrorLabel->setText(last_launched_process_or_error_.error().message().c_str());
  // }
}

void ProcessLauncherWidget::on_BrowseWorkingDirButton_clicked() {
  ORBIT_LOG("ProcessLauncherWidget::on_BrowseWorkingDirButton_clicked()");
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Open Directory"), "/home",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  ui->WorkingDirComboBox->lineEdit()->setText(dir);
}

QPushButton* ProcessLauncherWidget::GetLaunchButton() { return ui->LaunchButton; }

}  // namespace orbit_session_setup