// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/SessionSetupDialog.h"

#include <absl/flags/flag.h>

#include <QButtonGroup>
#include <QHistoryState>
#include <QNonConstOverload>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QWidget>
#include <Qt>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "ClientData/ProcessData.h"
#include "ClientFlags/ClientFlags.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/AddrAndPort.h"
#include "SessionSetup/ConnectToLocalWidget.h"
#include "SessionSetup/ConnectToSshWidget.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/DeploymentConfigurations.h"
#include "SessionSetup/LoadCaptureWidget.h"
#include "SessionSetup/OrbitServiceInstance.h"
#include "SessionSetup/ProcessListWidget.h"
#include "SessionSetup/TargetConfiguration.h"
#include "SessionSetup/TargetLabel.h"
#include "ui_SessionSetupDialog.h"

namespace orbit_session_setup {

using orbit_grpc_protos::ProcessInfo;

SessionSetupDialog::SessionSetupDialog(SshConnectionArtifacts* ssh_connection_artifacts,
                                       std::optional<TargetConfiguration> target_configuration_opt,
                                       QWidget* parent)
    : QDialog{parent, Qt::Window},
      ui_(std::make_unique<Ui::SessionSetupDialog>()),
      state_file_(&state_machine_),
      state_file_history_(&state_file_),
      state_file_selected_(&state_file_),
      state_file_no_selection_(&state_file_),
      state_local_(&state_machine_),
      state_local_history_(&state_local_),
      state_local_connecting_(&state_local_),
      state_local_connected_(&state_local_),
      state_local_no_process_selected_(&state_local_connected_),
      state_local_process_selected_(&state_local_connected_),
      state_ssh_(&state_machine_),
      state_ssh_history_(&state_ssh_),
      state_ssh_connecting_(&state_ssh_),
      state_ssh_connected_(&state_ssh_),
      state_ssh_no_process_selected_(&state_ssh_connected_),
      state_ssh_process_selected_(&state_ssh_connected_) {
  ORBIT_CHECK(ssh_connection_artifacts != nullptr);

  ui_->setupUi(this);
  ui_->localProfilingWidget->SetOrbitServiceInstanceCreateFunction(
      []() { return OrbitServiceInstance::CreatePrivileged(); });

  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
  SetupFileStates();
  SetupLocalStates();
  SetupSshStates();

  QObject::connect(ui_->confirmButton, &QPushButton::clicked, this, &QDialog::accept);
  QObject::connect(ui_->loadCaptureWidget, &LoadCaptureWidget::FileSelected, this,
                   [this](std::filesystem::path path) { selected_file_path_ = std::move(path); });
  QObject::connect(ui_->loadCaptureWidget, &LoadCaptureWidget::SelectionConfirmed, this,
                   &QDialog::accept);
  QObject::connect(ui_->processListWidget, &ProcessListWidget::ProcessSelected, this,
                   &SessionSetupDialog::UpdateTargetLabelWithProcess);
  QObject::connect(ui_->processListWidget, &ProcessListWidget::ProcessSelectionCleared,
                   ui_->targetLabel, &TargetLabel::Clear);
  QObject::connect(ui_->processListWidget, &ProcessListWidget::ProcessConfirmed, this,
                   &QDialog::accept);

  button_group_.addButton(ui_->localProfilingWidget->GetRadioButton());
  button_group_.addButton(ui_->loadCaptureWidget->GetRadioButton());
  button_group_.addButton(ui_->sshWidget->GetRadioButton());

  ui_->sshWidget->SetSshConnectionArtifacts(*ssh_connection_artifacts);

  if (target_configuration_opt.has_value()) {
    TargetConfiguration config = std::move(target_configuration_opt).value();
    std::visit([this](auto target) { SetTargetAndStateMachineInitialState(std::move(target)); },
               std::move(config));
    return;
  }

  ui_->processListWidget->SetProcessNameToSelect(absl::GetFlag(FLAGS_process_name));

  // If ssh_connection_artifacts contains a deployment method that is different that the default
  // (NoDeployment), it means they specified the deployment method via flags or environment
  // variables. In this case the "Connect to SSH" widget is pre-selected.
  if (!std::holds_alternative<NoDeployment>(
          *ssh_connection_artifacts->GetDeploymentConfiguration())) {
    state_machine_.setInitialState(&state_ssh_);
    ui_->sshWidget->GetRadioButton()->setChecked(true);
    return;
  }

  // Otherwise "Local Profiling" is pre-selected.
  state_machine_.setInitialState(&state_local_);
  ui_->localProfilingWidget->GetRadioButton()->setChecked(true);
}

SessionSetupDialog::~SessionSetupDialog() = default;

std::optional<TargetConfiguration> SessionSetupDialog::Exec() {
  state_machine_.start();

  int rc = QDialog::exec();
  state_machine_.stop();

  if (rc != QDialog::Accepted) return std::nullopt;

  if (state_machine_.configuration().contains(&state_local_)) {
    std::optional<ProcessInfo> process_info_opt = ui_->processListWidget->GetSelectedProcess();
    return LocalTarget(ui_->localProfilingWidget->TakeConnection(),
                       std::make_unique<orbit_client_data::ProcessData>(process_info_opt.value()));
  } else if (state_machine_.configuration().contains(&state_file_)) {
    return FileTarget(selected_file_path_);
  } else if (state_machine_.configuration().contains(&state_ssh_)) {
    std::optional<ProcessInfo> process_info_opt = ui_->processListWidget->GetSelectedProcess();
    return SshTarget(ui_->sshWidget->TakeConnection(),
                     std::make_unique<orbit_client_data::ProcessData>(process_info_opt.value()));
  } else {
    ORBIT_UNREACHABLE();
    return std::nullopt;
  }
}

void SessionSetupDialog::SetupSshStates() {
  // Setup initial and default
  state_ssh_.setInitialState(&state_ssh_connecting_);
  state_ssh_history_.setDefaultState(&state_ssh_connecting_);
  state_ssh_connected_.setInitialState(&state_ssh_no_process_selected_);

  // PROPERTIES
  // STATE state_ssh_
  state_ssh_.assignProperty(ui_->confirmButton, "enabled", false);
  state_ssh_.assignProperty(ui_->confirmButton, "toolTip",
                            "Please establish a SSH connection and select a process.");

  // STATE state_ssh_process_selected_
  state_ssh_process_selected_.assignProperty(ui_->confirmButton, "enabled", true);
  state_ssh_process_selected_.assignProperty(ui_->confirmButton, "toolTip", "");

  // TRANSITIONS (and entered/exit events)
  // STATE state_ssh_
  state_ssh_.addTransition(ui_->loadCaptureWidget->GetRadioButton(), &QRadioButton::clicked,
                           &state_file_history_);
  state_ssh_.addTransition(ui_->localProfilingWidget->GetRadioButton(), &QRadioButton::clicked,
                           &state_local_history_);

  // STATE state_ssh_connecting_
  state_ssh_connecting_.addTransition(ui_->sshWidget, &ConnectToSshWidget::Connected,
                                      &state_ssh_connected_);

  // STATE state_ssh_connected_
  state_ssh_connected_.addTransition(ui_->sshWidget, &ConnectToSshWidget::Disconnected,
                                     &state_ssh_connecting_);
  QObject::connect(&state_ssh_connected_, &QState::entered, this,
                   &SessionSetupDialog::ConnectSshAndProcessWidget);
  QObject::connect(&state_ssh_connected_, &QState::exited, this,
                   &SessionSetupDialog::DisconnectSshAndProcessWidget);

  // STATE state_ssh_no_process_selected_
  state_ssh_no_process_selected_.addTransition(
      ui_->processListWidget, &ProcessListWidget::ProcessSelected, &state_ssh_process_selected_);

  // STATE state_ssh_process_selected_
  state_ssh_process_selected_.addTransition(ui_->processListWidget,
                                            &ProcessListWidget::ProcessSelectionCleared,
                                            &state_ssh_no_process_selected_);
}

void SessionSetupDialog::SetupLocalStates() {
  // Setup initial and default
  state_local_.setInitialState(&state_local_connecting_);
  state_local_history_.setDefaultState(&state_local_connecting_);
  state_local_connected_.setInitialState(&state_local_no_process_selected_);

  // PROPERTIES
  // STATE state_local_
  state_local_.assignProperty(ui_->confirmButton, "enabled", false);
  state_local_.assignProperty(
      ui_->confirmButton, "toolTip",
      "Please have a OrbitService run on the local machine and select a process.");

  // STATE state_local_process_selected_
  state_local_process_selected_.assignProperty(ui_->confirmButton, "enabled", true);
  state_local_process_selected_.assignProperty(ui_->confirmButton, "toolTip", "");

  // TRANSITIONS (and entered/exit events)
  // STATE state_local_
  state_local_.addTransition(ui_->loadCaptureWidget->GetRadioButton(), &QRadioButton::clicked,
                             &state_file_history_);
  state_local_.addTransition(ui_->sshWidget->GetRadioButton(), &QRadioButton::clicked,
                             &state_ssh_history_);

  // STATE state_local_connecting_
  state_local_connecting_.addTransition(ui_->localProfilingWidget, &ConnectToLocalWidget::Connected,
                                        &state_local_connected_);

  // STATE state_local_connected_
  state_local_connected_.addTransition(
      ui_->localProfilingWidget, &ConnectToLocalWidget::Disconnected, &state_local_connecting_);
  QObject::connect(&state_local_connected_, &QState::entered, this,
                   &SessionSetupDialog::ConnectLocalAndProcessWidget);
  QObject::connect(&state_local_connected_, &QState::exited, this,
                   &SessionSetupDialog::DisconnectLocalAndProcessWidget);

  // STATE state_local_no_process_selected_
  state_local_no_process_selected_.addTransition(
      ui_->processListWidget, &ProcessListWidget::ProcessSelected, &state_local_process_selected_);

  // STATE state_local_process_selected_
  state_local_process_selected_.addTransition(ui_->processListWidget,
                                              &ProcessListWidget::ProcessSelectionCleared,
                                              &state_local_no_process_selected_);
}

void SessionSetupDialog::SetupFileStates() {
  // Setup initial and default
  state_file_.setInitialState(&state_file_no_selection_);
  state_file_history_.setDefaultState(&state_file_no_selection_);

  // PROPERTIES
  // STATE state_file_
  state_file_.assignProperty(ui_->confirmButton, "enabled", false);
  state_file_.assignProperty(ui_->confirmButton, "toolTip", "Please select a capture to load");
  state_file_.assignProperty(ui_->processListWidget, "enabled", false);

  // STATE state_file_selected_
  state_file_selected_.assignProperty(ui_->confirmButton, "enabled", true);
  state_file_selected_.assignProperty(ui_->confirmButton, "toolTip", "");

  // TRANSITIONS (and entered/exit events)
  // STATE state_file_
  state_file_.addTransition(ui_->localProfilingWidget->GetRadioButton(), &QRadioButton::clicked,
                            &state_local_history_);
  state_file_.addTransition(ui_->sshWidget->GetRadioButton(), &QRadioButton::clicked,
                            &state_ssh_history_);
  state_file_.addTransition(ui_->loadCaptureWidget, &LoadCaptureWidget::FileSelected,
                            &state_file_selected_);

  // STATE state_file_selected_
  QObject::connect(&state_file_selected_, &QState::entered, this,
                   [this] { ui_->targetLabel->ChangeToFileTarget(selected_file_path_); });
  QObject::connect(&state_file_selected_, &QState::exited, ui_->targetLabel, &TargetLabel::Clear);
}

void SessionSetupDialog::ConnectLocalAndProcessWidget() {
  QObject::connect(ui_->localProfilingWidget, &ConnectToLocalWidget::ProcessListUpdated,
                   ui_->processListWidget, &ProcessListWidget::UpdateList);
}

void SessionSetupDialog::DisconnectLocalAndProcessWidget() {
  ui_->processListWidget->Clear();
  QObject::disconnect(ui_->localProfilingWidget, &ConnectToLocalWidget::ProcessListUpdated,
                      ui_->processListWidget, &ProcessListWidget::UpdateList);
}

void SessionSetupDialog::ConnectSshAndProcessWidget() {
  QObject::connect(ui_->sshWidget, &ConnectToSshWidget::ProcessListUpdated, ui_->processListWidget,
                   &ProcessListWidget::UpdateList);
}

void SessionSetupDialog::DisconnectSshAndProcessWidget() {
  ui_->processListWidget->Clear();
  QObject::disconnect(ui_->sshWidget, &ConnectToSshWidget::ProcessListUpdated,
                      ui_->processListWidget, &ProcessListWidget::UpdateList);
}

void SessionSetupDialog::SetTargetAndStateMachineInitialState(SshTarget target) {
  ui_->processListWidget->SetProcessNameToSelect(target.process_->name());
  ui_->sshWidget->SetConnection(std::move(target.connection_));
  ui_->sshWidget->GetRadioButton()->setChecked(true);

  state_ssh_.setInitialState(&state_ssh_connected_);
  state_ssh_history_.setDefaultState(&state_ssh_connected_);
  state_machine_.setInitialState(&state_ssh_);
}

void SessionSetupDialog::SetTargetAndStateMachineInitialState(LocalTarget target) {
  ui_->processListWidget->SetProcessNameToSelect(target.process_->name());
  ui_->localProfilingWidget->SetConnection(std::move(target.connection_));
  ui_->localProfilingWidget->GetRadioButton()->setChecked(true);

  state_local_.setInitialState(&state_local_connected_);
  state_local_history_.setDefaultState(&state_local_connected_);
  state_machine_.setInitialState(&state_local_);
}

void SessionSetupDialog::SetTargetAndStateMachineInitialState(FileTarget target) {
  ui_->loadCaptureWidget->GetRadioButton()->setChecked(true);
  selected_file_path_ = target.GetCaptureFilePath();
  state_file_.setInitialState(&state_file_selected_);
  state_file_history_.setDefaultState(&state_file_selected_);
  state_machine_.setInitialState(&state_file_);
}

void SessionSetupDialog::UpdateTargetLabelWithProcess(
    const orbit_grpc_protos::ProcessInfo& process_info) {
  if (state_machine_.configuration().contains(&state_local_)) {
    ui_->targetLabel->ChangeToLocalTarget(process_info);
  } else if (state_machine_.configuration().contains(&state_ssh_)) {
    std::optional<orbit_ssh::AddrAndPort> addr_and_port = ui_->sshWidget->GetTargetAddrAndPort();
    ui_->targetLabel->ChangeToSshTarget(process_info, addr_and_port.value().GetHumanReadable());
  } else {
    ORBIT_UNREACHABLE();
  }
}

}  // namespace orbit_session_setup
