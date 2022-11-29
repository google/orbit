// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/SessionSetupDialog.h"

#include <absl/flags/flag.h>
#include <absl/time/time.h>

#include <QButtonGroup>
#include <QFrame>
#include <QHeaderView>
#include <QHistoryState>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMetaObject>
#include <QModelIndexList>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "ClientFlags/ClientFlags.h"
#include "ClientServices/ProcessManager.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "SessionSetup/ConnectToLocalWidget.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/LoadCaptureWidget.h"
#include "SessionSetup/OrbitServiceInstance.h"
#include "SessionSetup/OverlayWidget.h"
#include "SessionSetup/ProcessItemModel.h"
#include "SessionSetup/TargetConfiguration.h"
#include "SessionSetup/TargetLabel.h"
#include "ui_SessionSetupDialog.h"

namespace {
constexpr int kProcessesRowHeight = 19;
}  // namespace

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
      state_local_processes_loading_(&state_local_connected_),
      state_local_process_selected_(&state_local_connected_),
      state_local_no_process_selected_(&state_local_connected_) {
  ORBIT_CHECK(ssh_connection_artifacts != nullptr);

  ui_->setupUi(this);
  ui_->localProfilingWidget->SetOrbitServiceInstanceCreateFunction(
      []() { return OrbitServiceInstance::CreatePrivileged(); });
  ui_->processesTableOverlay->raise();

  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
  SetupFileStates();
  SetupLocalStates();

  process_proxy_model_.setSourceModel(&process_model_);
  process_proxy_model_.setSortRole(Qt::EditRole);
  process_proxy_model_.setFilterCaseSensitivity(Qt::CaseInsensitive);
  ui_->processesTableView->setModel(&process_proxy_model_);
  ui_->processesTableView->setSortingEnabled(true);
  ui_->processesTableView->sortByColumn(static_cast<int>(ProcessItemModel::Column::kCpu),
                                        Qt::DescendingOrder);

  ui_->processesTableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kPid), QHeaderView::ResizeToContents);
  ui_->processesTableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kCpu), QHeaderView::ResizeToContents);
  ui_->processesTableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kName), QHeaderView::Stretch);
  ui_->processesTableView->verticalHeader()->setDefaultSectionSize(kProcessesRowHeight);
  ui_->processesTableView->verticalHeader()->setVisible(false);

  QObject::connect(ui_->processesTableView->selectionModel(), &QItemSelectionModel::currentChanged,
                   this, &SessionSetupDialog::ProcessSelectionChanged);
  QObject::connect(ui_->processesTableView, &QTableView::doubleClicked, this, &QDialog::accept);
  QObject::connect(ui_->processFilterLineEdit, &QLineEdit::textChanged, &process_proxy_model_,
                   &QSortFilterProxyModel::setFilterFixedString);
  QObject::connect(ui_->confirmButton, &QPushButton::clicked, this, &QDialog::accept);
  QObject::connect(ui_->loadCaptureWidget, &LoadCaptureWidget::FileSelected, this,
                   [this](std::filesystem::path path) { selected_file_path_ = std::move(path); });
  QObject::connect(ui_->loadCaptureWidget, &LoadCaptureWidget::SelectionConfirmed, this,
                   &QDialog::accept);

  button_group_.addButton(ui_->localProfilingWidget->GetRadioButton());
  button_group_.addButton(ui_->loadCaptureWidget->GetRadioButton());

  if (target_configuration_opt.has_value()) {
    TargetConfiguration config = std::move(target_configuration_opt.value());
    target_configuration_opt = std::nullopt;
    std::visit([this](auto&& target) { SetTargetAndStateMachineInitialState(std::move(target)); },
               config);
  } else {
    state_machine_.setInitialState(&state_local_);
    ui_->localProfilingWidget->GetRadioButton()->setChecked(true);
  }
}

SessionSetupDialog::~SessionSetupDialog() = default;

std::optional<TargetConfiguration> SessionSetupDialog::Exec() {
  state_machine_.start();

  if (process_manager_ != nullptr) {
    process_manager_->SetProcessListUpdateListener(
        [this](std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
          OnProcessListUpdate(std::move(process_list));
        });
  }

  int rc = QDialog::exec();
  state_machine_.stop();

  if (rc != QDialog::Accepted) return std::nullopt;

  if (process_manager_ != nullptr) {
    process_manager_->SetProcessListUpdateListener(nullptr);
  }

  if (state_machine_.configuration().contains(&state_local_)) {
    return LocalTarget(ui_->localProfilingWidget->TakeConnection(), std::move(process_manager_),
                       std::move(process_));
  } else if (state_machine_.configuration().contains(&state_file_)) {
    return FileTarget(selected_file_path_);
  } else {
    ORBIT_UNREACHABLE();
    return std::nullopt;
  }
}

void SessionSetupDialog::ProcessSelectionChanged(const QModelIndex& current) {
  emit NoProcessSelected();

  if (!current.isValid()) {
    process_ = nullptr;
    return;
  }

  ORBIT_CHECK(current.data(Qt::UserRole).canConvert<const ProcessInfo*>());
  process_ = std::make_unique<orbit_client_data::ProcessData>(
      *current.data(Qt::UserRole).value<const ProcessInfo*>());
  emit ProcessSelected();
}

void SessionSetupDialog::SetupLocalStates() {
  // Setup initial and default
  state_local_.setInitialState(&state_local_connecting_);
  state_local_history_.setDefaultState(&state_local_connecting_);
  state_local_connected_.setInitialState(&state_local_processes_loading_);

  // PROPERTIES
  // STATE state_local_
  state_local_.assignProperty(ui_->confirmButton, "enabled", false);
  state_local_.assignProperty(
      ui_->confirmButton, "toolTip",
      "Please have a OrbitService run on the local machine and select a process.");

  // STATE state_local_connecting
  state_local_connecting_.assignProperty(ui_->processesFrame, "enabled", false);

  // STATE state_local_processes_loading_
  state_local_processes_loading_.assignProperty(ui_->processesTableOverlay, "visible", true);
  state_local_processes_loading_.assignProperty(ui_->processesTableOverlay, "cancelable", false);
  state_local_processes_loading_.assignProperty(ui_->processesTableOverlay, "statusMessage",
                                                "Loading processes...");

  // STATE state_local_process_selected_
  state_local_process_selected_.assignProperty(ui_->confirmButton, "enabled", true);
  state_local_process_selected_.assignProperty(ui_->confirmButton, "toolTip", "");

  // TRANSITIONS (and entered/exit events)
  // STATE state_local_
  state_local_.addTransition(ui_->loadCaptureWidget->GetRadioButton(), &QRadioButton::clicked,
                             &state_file_history_);

  // STATE state_local_connecting_
  state_local_connecting_.addTransition(ui_->localProfilingWidget, &ConnectToLocalWidget::Connected,
                                        &state_local_connected_);

  // STATE state_local_connected_
  state_local_connected_.addTransition(
      ui_->localProfilingWidget, &ConnectToLocalWidget::Disconnected, &state_local_connecting_);
  QObject::connect(&state_local_connected_, &QState::entered, this,
                   &SessionSetupDialog::SetupLocalProcessManager);
  QObject::connect(&state_local_connected_, &QState::exited, this,
                   &SessionSetupDialog::TearDownProcessManager);

  // STATE state_local_processes_loading_
  state_local_processes_loading_.addTransition(this, &SessionSetupDialog::ProcessSelected,
                                               &state_local_process_selected_);

  // STATE state_local_no_process_selected_
  state_local_no_process_selected_.addTransition(this, &SessionSetupDialog::ProcessSelected,
                                                 &state_local_process_selected_);

  // STATE state_local_process_selected_
  state_local_process_selected_.addTransition(this, &SessionSetupDialog::NoProcessSelected,
                                              &state_local_no_process_selected_);
  QObject::connect(&state_local_process_selected_, &QState::entered, this, [this] {
    ORBIT_CHECK(process_ != nullptr);
    ui_->targetLabel->ChangeToLocalTarget(*process_);
  });
  QObject::connect(&state_local_process_selected_, &QState::exited, ui_->targetLabel,
                   &TargetLabel::Clear);
}

void SessionSetupDialog::SetupFileStates() {
  // Setup initial and default
  state_file_.setInitialState(&state_file_no_selection_);
  state_file_history_.setDefaultState(&state_file_no_selection_);

  // PROPERTIES
  // STATE state_file_
  state_file_.assignProperty(ui_->confirmButton, "enabled", false);
  state_file_.assignProperty(ui_->confirmButton, "toolTip", "Please select a capture to load");
  state_file_.assignProperty(ui_->processesFrame, "enabled", false);

  // STATE state_file_selected_
  state_file_selected_.assignProperty(ui_->confirmButton, "enabled", true);
  state_file_selected_.assignProperty(ui_->confirmButton, "toolTip", "");

  // TRANSITIONS (and entered/exit events)
  // STATE state_file_
  state_file_.addTransition(ui_->localProfilingWidget->GetRadioButton(), &QRadioButton::clicked,
                            &state_local_history_);
  state_file_.addTransition(ui_->loadCaptureWidget, &LoadCaptureWidget::FileSelected,
                            &state_file_selected_);

  // STATE state_file_selected_
  QObject::connect(&state_file_selected_, &QState::entered, this,
                   [this] { ui_->targetLabel->ChangeToFileTarget(selected_file_path_); });
  QObject::connect(&state_file_selected_, &QState::exited, ui_->targetLabel, &TargetLabel::Clear);
}

void SessionSetupDialog::TearDownProcessManager() {
  process_model_.Clear();

  if (process_manager_ != nullptr) {
    process_manager_.reset();
  }
}

void SessionSetupDialog::SetupProcessManager(const std::shared_ptr<grpc::Channel>& grpc_channel) {
  ORBIT_CHECK(grpc_channel != nullptr);

  if (process_manager_ != nullptr) return;

  process_manager_ =
      orbit_client_services::ProcessManager::Create(grpc_channel, absl::Milliseconds(1000));
  process_manager_->SetProcessListUpdateListener(
      [this](std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
        OnProcessListUpdate(std::move(process_list));
      });
}

bool SessionSetupDialog::TrySelectProcessByName(const std::string& process_name) {
  QModelIndexList matches = process_proxy_model_.match(
      process_proxy_model_.index(0, static_cast<int>(ProcessItemModel::Column::kName)),
      Qt::DisplayRole, QVariant::fromValue(QString::fromStdString(process_name)));

  if (matches.isEmpty()) return false;

  ui_->processesTableView->selectionModel()->setCurrentIndex(
      matches[0], {QItemSelectionModel::SelectCurrent, QItemSelectionModel::Rows});
  return true;
}

void SessionSetupDialog::OnProcessListUpdate(
    std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
  QMetaObject::invokeMethod(this, [this, process_list = std::move(process_list)]() {
    bool had_processes_before = process_model_.HasProcesses();
    process_model_.SetProcesses(process_list);
    emit ProcessListUpdated();

    // In case there is a selection already, do not change anything, only update the cpu usage
    if (ui_->processesTableView->selectionModel()->hasSelection()) {
      const uint32_t selected_process_id = ui_->processesTableView->selectionModel()
                                               ->selectedRows()[0]
                                               .data(Qt::UserRole)
                                               .value<const ProcessInfo*>()
                                               ->pid();
      const auto it =
          std::find_if(process_list.begin(), process_list.end(),
                       [&](const auto& process) { return process.pid() == selected_process_id; });
      if (it != process_list.end()) {
        ui_->targetLabel->SetProcessCpuUsageInPercent(it->cpu_usage());
      }
      return;
    }

    // If process_ != nullptr here, that means it has been moved into the SessionSetupDialog and
    // the user was using this process before. TrySelectProcessByName attempts to select the process
    // again if it exists in process_model_.
    if (process_ != nullptr) {
      bool success = TrySelectProcessByName(process_->name());
      if (success) {
        ORBIT_LOG("Selected remembered process with name: %s", process_->name());
        return;
      }
    }

    if (!absl::GetFlag(FLAGS_process_name).empty()) {
      bool success = TrySelectProcessByName(absl::GetFlag(FLAGS_process_name));
      if (success) {
        ORBIT_LOG("Selected process with name: %s (provided via --process_name flag)",
                  absl::GetFlag(FLAGS_process_name));
        accept();
        return;
      }
    }

    // The first time a list of processes arrives, the cpu utilization values are not valid, since
    // they are computed as an average since the last time the list was refreshed. Hence return here
    // and do not perform a selection.
    if (!had_processes_before) return;

    // This selects the first (top most) row. The table is sorted by cpu usage (%) by default, so
    // unless the user changed the sorting, this will select the process with the highest cpu load
    ui_->processesTableView->selectRow(0);
  });
}

void SessionSetupDialog::SetupLocalProcessManager() {
  SetupProcessManager(ui_->localProfilingWidget->GetGrpcChannel());
}

void SessionSetupDialog::SetTargetAndStateMachineInitialState(SshTarget /*target*/) {
  ORBIT_FATAL("not implemented");
}

void SessionSetupDialog::SetTargetAndStateMachineInitialState(LocalTarget target) {
  ui_->localProfilingWidget->SetConnection(std::move(target.connection_));

  process_manager_ = std::move(target.process_manager_);
  process_ = std::move(target.process_);

  state_local_.setInitialState(&state_local_connected_);
  state_local_history_.setDefaultState(&state_local_connected_);
  state_machine_.setInitialState(&state_local_);
}

void SessionSetupDialog::SetTargetAndStateMachineInitialState(FileTarget target) {
  selected_file_path_ = target.GetCaptureFilePath();
  state_file_.setInitialState(&state_file_selected_);
  state_file_history_.setDefaultState(&state_file_selected_);
  state_machine_.setInitialState(&state_file_);
}

}  // namespace orbit_session_setup
