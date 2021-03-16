// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProfilingTargetDialog.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <grpc/impl/codegen/connectivity_state.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <QFileDialog>
#include <QFrame>
#include <QHeaderView>
#include <QHistoryState>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QModelIndexList>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <chrono>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "ConnectToStadiaWidget.h"
#include "Connections.h"
#include "OrbitBase/Logging.h"
#include "OrbitClientServices/ProcessManager.h"
#include "OverlayWidget.h"
#include "Path.h"
#include "ProcessItemModel.h"
#include "TargetConfiguration.h"
#include "process.pb.h"
#include "servicedeploymanager.h"
#include "ui_ProfilingTargetDialog.h"

ABSL_DECLARE_FLAG(bool, local);
ABSL_DECLARE_FLAG(std::string, process_name);

namespace {
constexpr int kProcessesRowHeight = 19;
}  // namespace

namespace orbit_qt {

using orbit_grpc_protos::ProcessInfo;

ProfilingTargetDialog::ProfilingTargetDialog(
    SshConnectionArtifacts* ssh_connection_artifacts,
    std::optional<TargetConfiguration> target_configuration_opt,
    orbit_metrics_uploader::MetricsUploader* metrics_uploader, QWidget* parent)
    : QDialog{parent, Qt::Window},
      ui_(std::make_unique<Ui::ProfilingTargetDialog>()),
      local_grpc_port_(ssh_connection_artifacts->GetGrpcPort().grpc_port),
      metrics_uploader_(metrics_uploader),
      state_stadia_(&state_machine_),
      state_stadia_history_(&state_stadia_),
      state_stadia_connecting_(&state_stadia_),
      state_stadia_connected_(&state_stadia_),
      state_stadia_processes_loading_(&state_stadia_connected_),
      state_stadia_process_selected_(&state_stadia_connected_),
      state_stadia_no_process_selected_(&state_stadia_connected_),
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
  CHECK(ssh_connection_artifacts != nullptr);

  ui_->setupUi(this);
  ui_->stadiaWidget->SetSshConnectionArtifacts(ssh_connection_artifacts);
  ui_->processesTableOverlay->raise();

  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
  SetupStadiaStates();
  SetupFileStates();
  SetupLocalStates();

  process_proxy_model_.setSourceModel(&process_model_);
  process_proxy_model_.setSortRole(Qt::EditRole);
  process_proxy_model_.setFilterCaseSensitivity(Qt::CaseInsensitive);
  ui_->processesTableView->setModel(&process_proxy_model_);
  ui_->processesTableView->setSortingEnabled(true);
  ui_->processesTableView->sortByColumn(static_cast<int>(ProcessItemModel::Column::kCpu),
                                        Qt::DescendingOrder);

  ui_->processesTableView->horizontalHeader()->resizeSection(
      static_cast<int>(ProcessItemModel::Column::kPid), 60);
  ui_->processesTableView->horizontalHeader()->resizeSection(
      static_cast<int>(ProcessItemModel::Column::kCpu), 60);
  ui_->processesTableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kName), QHeaderView::Stretch);
  ui_->processesTableView->verticalHeader()->setDefaultSectionSize(kProcessesRowHeight);
  ui_->processesTableView->verticalHeader()->setVisible(false);

  if (absl::GetFlag(FLAGS_local)) {
    ui_->localFrame->setVisible(true);
  }

  QObject::connect(ui_->loadFromFileButton, &QPushButton::clicked, this,
                   &ProfilingTargetDialog::SelectFile);
  // This and the next connect makes the radiobuttons behave as if they were in a exclusive button
  // group. If a user clicks on one of these and it was not checked before, it is checked afterwards
  // and the state machine transitions into the correct state, which "unchecks" the other buttons.
  // If a user clicks on a radio button that is already checked, simply nothing happens, the button
  // does not get unchecked.
  QObject::connect(ui_->loadCaptureRadioButton, &QRadioButton::clicked, this, [this](bool checked) {
    if (!checked) ui_->loadCaptureRadioButton->setChecked(true);
  });
  QObject::connect(ui_->localProfilingRadioButton, &QRadioButton::clicked, this,
                   [this](bool checked) {
                     if (!checked) ui_->localProfilingRadioButton->setChecked(true);
                   });
  QObject::connect(ui_->processesTableView->selectionModel(), &QItemSelectionModel::currentChanged,
                   this, &ProfilingTargetDialog::ProcessSelectionChanged);
  QObject::connect(ui_->processesTableView, &QTableView::doubleClicked, this, &QDialog::accept);
  QObject::connect(ui_->processFilterLineEdit, &QLineEdit::textChanged, &process_proxy_model_,
                   &QSortFilterProxyModel::setFilterFixedString);
  QObject::connect(ui_->confirmButton, &QPushButton::clicked, this, &QDialog::accept);

  if (target_configuration_opt.has_value()) {
    TargetConfiguration config = std::move(target_configuration_opt.value());
    target_configuration_opt = std::nullopt;
    SetStateMachineInitialStateFromTarget(std::move(config));
  } else {
    SetStateMachineInitialState();
  }

  if (metrics_uploader_ != nullptr) {
    metrics_uploader_->SendLogEvent(
        orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_SESSION_SETUP_WINDOW_OPEN);
  }
}

void ProfilingTargetDialog::SetStateMachineInitialState() {
  if (absl::GetFlag(FLAGS_local)) {
    state_machine_.setInitialState(&state_local_);
  } else if (ui_->stadiaWidget->IsActive()) {
    state_machine_.setInitialState(&state_stadia_);
  } else {
    state_machine_.setInitialState(&state_file_);
  }
}

ProfilingTargetDialog::~ProfilingTargetDialog() noexcept = default;

std::optional<TargetConfiguration> ProfilingTargetDialog::Exec() {
  ui_->stadiaWidget->Start();
  state_machine_.start();
  int rc = QDialog::exec();
  state_machine_.stop();

  if (metrics_uploader_ != nullptr) {
    metrics_uploader_->SendLogEvent(
        orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_SESSION_SETUP_WINDOW_CLOSE);
  }

  if (rc != QDialog::Accepted) return std::nullopt;

  if (process_manager_ != nullptr) {
    process_manager_->SetProcessListUpdateListener(nullptr);
  }

  if (state_machine_.configuration().contains(&state_stadia_)) {
    return StadiaTarget(ui_->stadiaWidget->StopAndClearConnection().value(),
                        std::move(process_manager_), std::move(process_));
  } else if (state_machine_.configuration().contains(&state_local_)) {
    return LocalTarget(LocalConnection(std::move(local_grpc_channel_)), std::move(process_manager_),
                       std::move(process_));
  } else if (state_machine_.configuration().contains(&state_file_)) {
    return FileTarget(selected_file_path_);
  } else {
    UNREACHABLE();
    return std::nullopt;
  }
}

void ProfilingTargetDialog::ProcessSelectionChanged(const QModelIndex& current) {
  emit NoProcessSelected();

  if (!current.isValid()) {
    process_ = nullptr;
    return;
  }

  CHECK(current.data(Qt::UserRole).canConvert<const ProcessInfo*>());
  process_ = std::make_unique<ProcessData>(*current.data(Qt::UserRole).value<const ProcessInfo*>());
  emit ProcessSelected();
}

void ProfilingTargetDialog::SetupStadiaStates() {
  // Setup initial and default
  state_stadia_.setInitialState(&state_stadia_connecting_);
  state_stadia_history_.setDefaultState(&state_stadia_connecting_);
  state_stadia_connected_.setInitialState(&state_stadia_processes_loading_);

  // PROPERTIES
  // STATE state_stadia_
  state_stadia_.assignProperty(ui_->confirmButton, "enabled", false);
  state_stadia_.assignProperty(ui_->confirmButton, "toolTip",
                               "Please connect to an instance and select a process.");
  state_stadia_.assignProperty(ui_->stadiaWidget, "active", true);
  state_stadia_.assignProperty(ui_->loadCaptureRadioButton, "checked", false);
  state_stadia_.assignProperty(ui_->localProfilingRadioButton, "checked", false);
  state_stadia_.assignProperty(ui_->targetLabel, "text", "");

  // STATE state_stadia_connecting_
  state_stadia_connecting_.assignProperty(ui_->processesFrame, "enabled", false);

  // STATE state_stadia_processes_loading_
  state_stadia_processes_loading_.assignProperty(ui_->processesTableOverlay, "visible", true);
  state_stadia_processes_loading_.assignProperty(ui_->processesTableOverlay, "cancelable", false);
  state_stadia_processes_loading_.assignProperty(ui_->processesTableOverlay, "statusMessage",
                                                 "Loading processes...");

  // STATE state_stadia_process_selected_
  state_stadia_process_selected_.assignProperty(ui_->confirmButton, "enabled", true);
  state_stadia_process_selected_.assignProperty(ui_->confirmButton, "toolTip", "");

  // TRANSITIONS (and entered/exit events)
  // STATE state_stadia_
  state_stadia_.addTransition(ui_->loadCaptureRadioButton, &QRadioButton::clicked,
                              &state_file_history_);
  state_stadia_.addTransition(ui_->localProfilingRadioButton, &QRadioButton::clicked,
                              &state_local_history_);
  state_stadia_.addTransition(ui_->stadiaWidget, &ConnectToStadiaWidget::Disconnected,
                              &state_stadia_connecting_);

  // STATE state_stadia_connecting_
  state_stadia_connecting_.addTransition(ui_->stadiaWidget, &ConnectToStadiaWidget::Connected,
                                         &state_stadia_connected_);
  state_stadia_connecting_.addTransition(this, &ProfilingTargetDialog::StadiaIsConnected,
                                         &state_stadia_connected_);
  QObject::connect(&state_stadia_connecting_, &QState::entered, this, [this]() {
    if (ui_->stadiaWidget->GetGrpcChannel() != nullptr) {
      emit StadiaIsConnected();
    }
  });

  // STATE state_stadia_connected_
  QObject::connect(&state_stadia_connected_, &QState::entered, this,
                   &ProfilingTargetDialog::SetupStadiaProcessManager);
  QObject::connect(&state_stadia_connected_, &QState::exited, this,
                   &ProfilingTargetDialog::TearDownProcessManager);

  // STATE state_stadia_processes_loading_
  state_stadia_processes_loading_.addTransition(this, &ProfilingTargetDialog::ProcessSelected,
                                                &state_stadia_process_selected_);

  // STATE state_stadia_no_process_selected_
  state_stadia_no_process_selected_.addTransition(this, &ProfilingTargetDialog::ProcessSelected,
                                                  &state_stadia_process_selected_);

  // STATE s_s_process_selected
  state_stadia_process_selected_.addTransition(this, &ProfilingTargetDialog::NoProcessSelected,
                                               &state_stadia_no_process_selected_);
  QObject::connect(&state_stadia_process_selected_, &QState::entered, [this] {
    CHECK(process_ != nullptr);
    QString process_name = QString::fromStdString(process_->name());
    CHECK(ui_->stadiaWidget->GetSelectedInstance().has_value());
    QString instance_name = ui_->stadiaWidget->GetSelectedInstance().value().display_name;
    ui_->targetLabel->setText(QString("%1 @ %2").arg(process_name).arg(instance_name));
  });
}

void ProfilingTargetDialog::SetupLocalStates() {
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
  state_local_.assignProperty(ui_->localProfilingRadioButton, "checked", true);
  state_local_.assignProperty(ui_->stadiaWidget, "active", false);
  state_local_.assignProperty(ui_->loadCaptureRadioButton, "checked", false);
  state_local_.assignProperty(ui_->targetLabel, "text", "");

  // STATE state_local_connecting_
  state_local_connecting_.assignProperty(ui_->localProfilingStatusMessage, "text", "Connecting...");
  state_local_connecting_.assignProperty(
      ui_->localProfilingStatusMessage, "toolTip",
      "Orbit is trying to connect to a local OrbitService. Please make sure OrbitService is "
      "running on the local machine.");

  // STATE state_local_connected_
  state_local_connected_.assignProperty(ui_->localProfilingStatusMessage, "text", "Connected");

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
  state_local_.addTransition(ui_->stadiaWidget, &ConnectToStadiaWidget::Activated,
                             &state_stadia_history_);
  state_local_.addTransition(ui_->loadCaptureRadioButton, &QRadioButton::clicked,
                             &state_file_history_);

  // STATE state_local_connecting_
  state_local_connecting_.addTransition(this, &ProfilingTargetDialog::ProcessListUpdated,
                                        &state_local_connected_);
  QObject::connect(&state_local_connecting_, &QState::entered, this,
                   &ProfilingTargetDialog::ConnectToLocal);

  // STATE state_local_connected_
  QObject::connect(&state_local_connected_, &QState::entered, this,
                   &ProfilingTargetDialog::SetupLocalProcessManager);
  QObject::connect(&state_local_connected_, &QState::exited, this,
                   &ProfilingTargetDialog::TearDownProcessManager);

  // STATE state_local_processes_loading_
  state_local_processes_loading_.addTransition(this, &ProfilingTargetDialog::ProcessSelected,
                                               &state_local_process_selected_);

  // STATE state_local_no_process_selected_
  state_local_no_process_selected_.addTransition(this, &ProfilingTargetDialog::ProcessSelected,
                                                 &state_local_process_selected_);

  // STATE state_local_process_selected_
  state_local_process_selected_.addTransition(this, &ProfilingTargetDialog::NoProcessSelected,
                                              &state_local_no_process_selected_);
  QObject::connect(&state_local_process_selected_, &QState::entered, [this] {
    CHECK(process_ != nullptr);
    QString process_name = QString::fromStdString(process_->name());
    ui_->targetLabel->setText(QString("%1 @ localhost").arg(process_name));
  });
}

void ProfilingTargetDialog::SetStateMachineInitialStateFromTarget(TargetConfiguration config) {
  std::visit([this](auto target) { SetTargetAndStateMachineInitialState(std::move(target)); },
             std::move(config));

  if (process_manager_ != nullptr) {
    process_manager_->SetProcessListUpdateListener(
        [this](std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
          OnProcessListUpdate(std::move(process_list));
        });
  }
}

void ProfilingTargetDialog::SetupFileStates() {
  // Setup initial and default
  state_file_.setInitialState(&state_file_no_selection_);
  state_file_history_.setDefaultState(&state_file_no_selection_);

  // PROPERTIES
  // STATE state_file_
  state_file_.assignProperty(ui_->confirmButton, "enabled", false);
  state_file_.assignProperty(ui_->confirmButton, "toolTip", "Please select a capture to load");
  state_file_.assignProperty(ui_->stadiaWidget, "active", false);
  state_file_.assignProperty(ui_->loadCaptureRadioButton, "checked", true);
  state_file_.assignProperty(ui_->processesFrame, "enabled", false);
  state_file_.assignProperty(ui_->loadFromFileButton, "enabled", true);
  state_file_.assignProperty(ui_->localProfilingRadioButton, "checked", false);
  state_file_.assignProperty(ui_->targetLabel, "text", "");

  // STATE state_file_selected_
  state_file_selected_.assignProperty(ui_->confirmButton, "enabled", true);
  state_file_selected_.assignProperty(ui_->confirmButton, "toolTip", "");

  // TRANSITIONS (and entered/exit events)
  // STATE state_file_
  state_file_.addTransition(ui_->stadiaWidget, &ConnectToStadiaWidget::Activated,
                            &state_stadia_history_);
  state_file_.addTransition(ui_->localProfilingRadioButton, &QRadioButton::clicked,
                            &state_local_history_);
  state_file_.addTransition(this, &ProfilingTargetDialog::FileSelected, &state_file_selected_);

  // STATE state_file_no_selection_
  QObject::connect(&state_file_no_selection_, &QState::entered, this, [this] {
    if (selected_file_path_.empty()) SelectFile();
  });

  // STATE state_file_selected_
  QObject::connect(&state_file_selected_, &QState::entered, this, [this] {
    QString filename = QString::fromStdString(selected_file_path_.filename().string());
    ui_->selectedFileLabel->setText(filename);
    ui_->targetLabel->setText(filename);
  });
}

void ProfilingTargetDialog::TearDownProcessManager() {
  process_model_.Clear();

  if (process_manager_ != nullptr) {
    process_manager_.reset();
  }
}

void ProfilingTargetDialog::SetupProcessManager(
    const std::shared_ptr<grpc::Channel>& grpc_channel) {
  CHECK(grpc_channel != nullptr);

  if (process_manager_ != nullptr) return;

  process_manager_ = ProcessManager::Create(grpc_channel, absl::Milliseconds(1000));
  process_manager_->SetProcessListUpdateListener(
      [this](std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
        OnProcessListUpdate(std::move(process_list));
      });
}

void ProfilingTargetDialog::SetupStadiaProcessManager() {
  SetupProcessManager(ui_->stadiaWidget->GetGrpcChannel());
}

void ProfilingTargetDialog::SelectFile() {
  const QString file = QFileDialog::getOpenFileName(
      this, "Open Capture...", QString::fromStdString(Path::CreateOrGetCaptureDir().string()),
      "*.orbit");
  if (!file.isEmpty()) {
    selected_file_path_ = std::filesystem::path(file.toStdString());

    emit FileSelected();
  }
}

bool ProfilingTargetDialog::TrySelectProcess(const std::string& process_name) {
  QModelIndexList matches = process_proxy_model_.match(
      process_proxy_model_.index(0, static_cast<int>(ProcessItemModel::Column::kName)),
      Qt::DisplayRole, QVariant::fromValue(QString::fromStdString(process_name)));

  if (matches.isEmpty()) return false;

  LOG("Selecting remembered process: %s", process_name);

  ui_->processesTableView->selectionModel()->setCurrentIndex(
      matches[0], {QItemSelectionModel::SelectCurrent, QItemSelectionModel::Rows});
  return true;
}

void ProfilingTargetDialog::OnProcessListUpdate(
    std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
  QMetaObject::invokeMethod(this, [this, process_list = std::move(process_list)]() {
    bool had_processes_before = process_model_.HasProcesses();
    process_model_.SetProcesses(process_list);
    emit ProcessListUpdated();

    // In case there is a selection already, do not change anything
    if (ui_->processesTableView->selectionModel()->hasSelection()) return;

    // If process_ != nullptr here, that means it has been moved into the
    // ProfilingTargetDialog and the user was using this process before.
    // TrySelectProcess attempts to select the process again if it exists
    // in process_model_.
    if (process_ != nullptr) {
      bool success = TrySelectProcess(process_->name());
      if (success) return;
    }

    if (absl::GetFlag(FLAGS_process_name) != "") {
      bool success = TrySelectProcess(absl::GetFlag(FLAGS_process_name));
      if (success) accept();
    }

    // The first time a list of processes arrives, the cpu utilization
    // values are not valid, since they are computed as an average since
    // the last time the list was refreshed. Hence return here and do not
    // perform a selection.
    if (!had_processes_before) return;

    // This selects the first (top most) row. The table is sorted by cpu
    // usage (%) by default, so unless the user changed the sorting, this
    // will select the process with the highest cpu load
    ui_->processesTableView->selectRow(0);
  });
}

void ProfilingTargetDialog::ConnectToLocal() {
  if (local_grpc_channel_ == nullptr) {
    local_grpc_channel_ =
        grpc::CreateCustomChannel(absl::StrFormat("127.0.0.1:%d", local_grpc_port_),
                                  grpc::InsecureChannelCredentials(), grpc::ChannelArguments());
  }

  SetupLocalProcessManager();
}

void ProfilingTargetDialog::SetupLocalProcessManager() { SetupProcessManager(local_grpc_channel_); }

void ProfilingTargetDialog::SetTargetAndStateMachineInitialState(StadiaTarget target) {
  state_machine_.setInitialState(&state_stadia_);

  if (target.connection_.GetGrpcChannel()->GetState(false) != GRPC_CHANNEL_READY) return;

  ui_->stadiaWidget->SetConnection(std::move(target.connection_));
  process_manager_ = std::move(target.process_manager_);
  process_ = std::move(target.process_);
  state_stadia_.setInitialState(&state_stadia_connected_);
  state_stadia_history_.setDefaultState(&state_stadia_connected_);
}

void ProfilingTargetDialog::SetTargetAndStateMachineInitialState(LocalTarget target) {
  local_grpc_channel_ = target.GetConnection()->GetGrpcChannel();
  process_manager_ = std::move(target.process_manager_);
  process_ = std::move(target.process_);

  state_local_.setInitialState(&state_local_connected_);
  state_local_history_.setDefaultState(&state_local_connected_);
  state_machine_.setInitialState(&state_local_);
}

void ProfilingTargetDialog::SetTargetAndStateMachineInitialState(FileTarget target) {
  selected_file_path_ = target.GetCaptureFilePath();
  state_file_.setInitialState(&state_file_selected_);
  state_file_history_.setDefaultState(&state_file_selected_);
  state_machine_.setInitialState(&state_file_);
}

}  // namespace orbit_qt