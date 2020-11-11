// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProfilingTargetDialog.h"

#include <QApplication>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <memory>
#include <optional>
#include <variant>

#include "ConnectionArtifacts.h"
#include "Error.h"
#include "MainThreadExecutor.h"
#include "OrbitClientServices/ProcessManager.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/InstanceItemModel.h"
#include "OrbitGgp/SshInfo.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSshQt/ScopedConnection.h"
#include "OverlayWidget.h"
#include "Path.h"
#include "ProcessItemModel.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "deploymentconfigurations.h"
#include "grpcpp/grpcpp.h"
#include "process.pb.h"
#include "qheaderview.h"
#include "servicedeploymanager.h"

namespace {
const QString kRememberChosenInstance{"RememberChosenInstance"};
}  // namespace

namespace OrbitQt {

using orbit_grpc_protos::ProcessInfo;
using OrbitGgp::Instance;
using OrbitSshQt::ScopedConnection;

ProfilingTargetDialog::ProfilingTargetDialog(ConnectionArtifacts* connection_artifacts,
                                             MainThreadExecutor* main_thread_executer,
                                             QWidget* parent)
    : QDialog(parent),
      ui_(std::make_unique<Ui::ProfilingTargetDialog>()),
      connection_artifacts_(connection_artifacts),
      main_thread_executer_(main_thread_executer),
      settings_("The Orbit Authors", "Orbit Profiler") {
  CHECK(connection_artifacts_ != nullptr);
  CHECK(main_thread_executer_ != nullptr);

  SetupUi();
}

std::variant<std::monostate, const ConnectionArtifacts*, QString> ProfilingTargetDialog::Exec() {
  ResizeTables();
  on_connectToStadiaInstanceRadioButton_toggled(
      ui_->connectToStadiaInstanceRadioButton->isChecked());
  on_loadCaptureRadioButton_toggled(ui_->loadCaptureRadioButton->isChecked());

  int dialog_return_code = exec();

  if (dialog_return_code == 0) {
    return std::monostate();
  }

  return dialog_result_;
}

void ProfilingTargetDialog::SetupUi() {
  ui_->setupUi(this);

  if (!settings_.value(kRememberChosenInstance).toString().isEmpty()) {
    ui_->rememberCheckBox->setChecked(true);
  }

  CHECK(ggp_client_ == nullptr);
  auto client_result = OrbitGgp::Client::Create(this);
  if (!client_result) {
    ui_->connectToStadiaInstanceRadioButton->setToolTip(
        QString::fromStdString(client_result.error().message()));
    return;
  }
  ggp_client_ = client_result.value();

  ui_->connectToStadiaInstanceRadioButton->setEnabled(true);
  ui_->connectToStadiaInstanceRadioButton->setChecked(true);

  ui_->instancesTableView->setModel(&instance_model_);
  ui_->instancesTableView->setEnabled(true);
  QObject::connect(ui_->instancesTableView->selectionModel(), &QItemSelectionModel::currentChanged,
                   this, [this](const QModelIndex& current) {
                     if (!current.isValid()) {
                       connection_artifacts_->selected_instance_ = std::nullopt;
                       ui_->connectButton->setEnabled(false);
                       return;
                     }

                     CHECK(current.model() == &instance_model_);
                     connection_artifacts_->selected_instance_ =
                         current.data(Qt::UserRole).value<Instance>();
                     ui_->connectButton->setEnabled(true);
                   });

  process_proxy_model_.setSourceModel(&process_model_);
  process_proxy_model_.setSortRole(Qt::EditRole);
  ui_->processesTableView->setModel(&process_proxy_model_);
  ui_->processesTableView->setSortingEnabled(true);
  ui_->processesTableView->sortByColumn(static_cast<int>(ProcessItemModel::Column::kCpu),
                                        Qt::DescendingOrder);

  ui_->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

  QObject::connect(
      ui_->processesTableView->selectionModel(), &QItemSelectionModel::currentChanged, this,
      [this](const QModelIndex& current) {
        if (!current.isValid()) {
          connection_artifacts_->process_ = nullptr;
          ui_->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
          //  ui_->selectProcessButton->setEnabled(false)
          return;
        }

        CHECK(current.model() == &process_proxy_model_);
        const auto* process_info = current.data(Qt::UserRole).value<const ProcessInfo*>();
        connection_artifacts_->process_ = std::make_unique<ProcessData>(*process_info);
        // on_selectProcessButton_clicked();
        dialog_result_ = connection_artifacts_;
        // ui_->selectProcessButton->setEnabled(true);
        this->EnableConfirm("Select Process");
      });

  QObject::connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  QObject::connect(ui_->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  ReloadInstances();
}

void ProfilingTargetDialog::ConnectToInstance() {
  CHECK(connection_artifacts_->selected_instance_);

  const QString& instance_id = connection_artifacts_->selected_instance_->id;
  const QString& instance_name = connection_artifacts_->selected_instance_->display_name;

  if (ui_->rememberCheckBox->isChecked()) {
    settings_.setValue(kRememberChosenInstance, instance_id);
  }

  connect_clicked_ = true;
  ui_->refreshButton->setEnabled(false);
  ui_->connectButton->setEnabled(false);
  ui_->instancesTableOverlay->Activate(QString{"Connecting to Instance %1 ..."}.arg(instance_name),
                                       [this]() { connect_clicked_ = false; });

  if (instance_credentials_.contains(instance_id.toStdString())) {
    DeployOrbitService();
  } else {
    ui_->instancesTableOverlay->UpdateMessage(
        QString{"Loading encryption credentials for Instance %1 ..."}.arg(instance_name));
  }
}

void ProfilingTargetDialog::DisconnectFromInstance() {
  connection_artifacts_->process_ = nullptr;
  process_model_.Clear();
  if (connection_artifacts_->process_manager_ != nullptr) {
    connection_artifacts_->process_manager_->Shutdown();
    connection_artifacts_->process_manager_ = nullptr;
  }
  ui_->processesTableView->setEnabled(false);
  // ui_->selectProcessButton->setEnabled(false);
  ui_->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

  connection_artifacts_->grpc_channel_ = nullptr;

  // TODO (antonrohr) (this currently does not work)
  // if (connection_artifacts_->service_deploy_manager_ != nullptr)
  // connection_artifacts_->service_deploy_manager_->Shutdown()
  connection_artifacts_->service_deploy_manager_ = nullptr;

  ui_->instancesTableOverlay->Deactivate();
  ui_->refreshButton->setEnabled(true);
  ui_->instancesTableView->setEnabled(true);

  if (connection_artifacts_->selected_instance_) {
    ui_->connectButton->setEnabled(true);
  }
}

void ProfilingTargetDialog::DeployOrbitService() {
  CHECK(connection_artifacts_->service_deploy_manager_ == nullptr);
  CHECK(connection_artifacts_->selected_instance_);
  const std::string instance_id = connection_artifacts_->selected_instance_->id.toStdString();
  CHECK(instance_credentials_.contains(instance_id));

  connect_clicked_ = false;

  const OrbitSsh::Credentials& credentials{instance_credentials_.at(instance_id)};

  connection_artifacts_->CreateServiceDeployManager(credentials);

  ScopedConnection label_connection{QObject::connect(
      connection_artifacts_->service_deploy_manager_.get(), &ServiceDeployManager::statusMessage,
      ui_->instancesTableOverlay, &OverlayWidget::UpdateMessage)};

  ui_->instancesTableOverlay->UpdateCancelButton(
      [this]() { connection_artifacts_->service_deploy_manager_->Cancel(); });

  const auto deployment_result = connection_artifacts_->service_deploy_manager_->Exec();
  if (!deployment_result) {
    DisconnectFromInstance();

    if (deployment_result.error() != make_error_code(Error::kUserCanceledServiceDeployment)) {
      DisplayErrorToUser(
          QString(
              "Orbit was unable to successfully connect to the Instance. The error message was: %1")
              .arg(QString::fromStdString(deployment_result.error().message())));
    }
    return;
  }
  LOG("Deployment successful, grpc_port: %d", deployment_result.value().grpc_port);
  ui_->instancesTableOverlay->UpdateMessage(
      QString{"Connected to %1"}.arg(connection_artifacts_->selected_instance_->display_name));
  ui_->instancesTableOverlay->StopSpinner();
  ui_->instancesTableOverlay->UpdateCancelButton([this]() { DisconnectFromInstance(); },
                                                 "Disconnect");

  CHECK(connection_artifacts_->grpc_channel_ == nullptr);
  std::string grpc_server_address =
      absl::StrFormat("127.0.0.1:%d", deployment_result.value().grpc_port);
  LOG("Starting gRPC channel to: %s", grpc_server_address);
  connection_artifacts_->grpc_channel_ = grpc::CreateCustomChannel(
      grpc_server_address, grpc::InsecureChannelCredentials(), grpc::ChannelArguments());
  CHECK(connection_artifacts_->grpc_channel_ != nullptr);

  CHECK(connection_artifacts_->process_manager_ == nullptr);
  ui_->processesTableOverlay->Activate("Loading Processes");
  connection_artifacts_->process_manager_ =
      ProcessManager::Create(connection_artifacts_->grpc_channel_, absl::Milliseconds(1000));
  connection_artifacts_->process_manager_->SetProcessListUpdateListener(
      [this](ProcessManager* process_manager) {
        main_thread_executer_->Schedule([this, process_manager]() {
          process_model_.SetProcesses(process_manager->GetProcessList());
          // ui_->processesTableView->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
          // LOG("row 0 height %d", ui_->processesTableView->rowHeight(0));
          ui_->processesTableView->setEnabled(true);
          ui_->processesTableOverlay->Deactivate();
        });
      });
}

void ProfilingTargetDialog::ReloadInstances() {
  ui_->refreshButton->setEnabled(false);
  ui_->connectButton->setEnabled(false);
  ui_->instancesTableOverlay->Activate("Loading Instances...");

  ggp_client_->GetInstancesAsync([this](outcome::result<QVector<Instance>> instances) {
    ui_->refreshButton->setEnabled(true);
    ui_->instancesTableOverlay->Deactivate();

    if (!instances) {
      DisplayErrorToUser(QString("Orbit was unable to retrieve the list of available Stadia "
                                 "instances. The error message was: %1")
                             .arg(QString::fromStdString(instances.error().message())));
      return;
    }

    instance_model_.SetInstances(instances.value());

    if (connection_artifacts_->selected_instance_ &&
        ui_->connectToStadiaInstanceRadioButton->isChecked()) {
      ui_->connectButton->setEnabled(true);
    }

    const QString remembered_instance_id = settings_.value(kRememberChosenInstance).toString();
    if (!remembered_instance_id.isEmpty()) {
      int row = instance_model_.GetRowOfInstanceById(remembered_instance_id);
      if (row != -1) {
        ui_->instancesTableView->selectRow(row);
        ConnectToInstance();
      }
    }

    for (auto& instance : instances.value()) {
      std::string instance_id = instance.id.toStdString();

      if (instance_credentials_.contains(instance_id)) continue;

      ggp_client_->GetSshInfoAsync(
          instance, [this, instance_id = std::move(instance_id)](
                        outcome::result<OrbitGgp::SshInfo> ssh_info_result) {
            if (!ssh_info_result) {
              ERROR("'ggp ssh init' call failed for instance with id %s", instance_id);
              DisplayErrorToUser(
                  QString("Unable to load encryption credentials for instance with id %1")
                      .arg(QString::fromStdString(instance_id)));
              return;
            }
            LOG("Received ssh info for instance with id: %s", instance_id);

            OrbitGgp::SshInfo& ssh_info{ssh_info_result.value()};
            OrbitSsh::Credentials credentials;
            credentials.addr_and_port = {ssh_info.host.toStdString(), ssh_info.port};
            credentials.key_path = ssh_info.key_path.toStdString();
            credentials.known_hosts_path = ssh_info.known_hosts_path.toStdString();
            credentials.user = ssh_info.user.toStdString();

            instance_credentials_[instance_id] = std::move(credentials);

            if (connect_clicked_) {
              ConnectToInstance();
            }
          });
    }
  });
}

void ProfilingTargetDialog::on_loadCaptureRadioButton_toggled(bool checked) {
  ui_->loadFromFileButton->setEnabled(checked);
  // ui_->captureTableView->setEnabled(checked);
}

void ProfilingTargetDialog::on_connectToStadiaInstanceRadioButton_toggled(bool checked) {
  ui_->instancesTableView->setEnabled(checked);
  ui_->instancesTableOverlay->setEnabled(checked);
  ui_->processesFrame->setEnabled(checked);
  ui_->rememberCheckBox->setEnabled(checked);

  if (!checked) {
    ui_->refreshButton->setEnabled(false);
    ui_->connectButton->setEnabled(false);
    return;
  }

  if (ui_->instancesTableOverlay->isVisible()) return;

  ui_->refreshButton->setEnabled(true);
  if (connection_artifacts_->selected_instance_) {
    ui_->connectButton->setEnabled(true);
  }
}

void ProfilingTargetDialog::on_selectProcessButton_clicked() {
  CHECK(connection_artifacts_->process_ != nullptr);
  dialog_result_ = connection_artifacts_;
  accept();
}

void ProfilingTargetDialog::on_loadFromFileButton_clicked() {
  const QString file = QFileDialog::getOpenFileName(
      this, "Open Capture...", QString::fromStdString(Path::CreateOrGetCaptureDir()), "*.orbit");
  if (!file.isEmpty()) {
    file_to_load_ = file;
    dialog_result_ = file;
    ui_->chosen_file_label->setText(file);
    EnableConfirm("Load Capture");
    // DisconnectFromInstance();
    // accept();
  }
}

void ProfilingTargetDialog::on_rememberCheckBox_toggled(bool checked) {
  if (!checked) {
    settings_.remove(kRememberChosenInstance);
    return;
  }

  if (connection_artifacts_->selected_instance_) {
    settings_.setValue(kRememberChosenInstance, connection_artifacts_->selected_instance_->id);
  }
}

void ProfilingTargetDialog::DisplayErrorToUser(const QString& message) {
  QMessageBox::critical(this, QApplication::applicationName(), message);
}

void ProfilingTargetDialog::EnableConfirm(const QString& text) {
  ui_->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText(text);
  ui_->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
}

void ProfilingTargetDialog::ResizeTables() {
  LOG("table width %d", ui_->processesTableView->size().width());
  // ui_->procesasesTableView->horizontalHeader()->setSectionResizeMode()
  ui_->processesTableView->horizontalHeader()->resizeSection(
      static_cast<int>(ProcessItemModel::Column::kPid), 60);
  ui_->processesTableView->horizontalHeader()->resizeSection(
      static_cast<int>(ProcessItemModel::Column::kCpu), 60);
  // ui_->processesTableView->horizontalHeader()->resizeSection(
  //     static_cast<int>(ProcessItemModel::Column::kName),
  //     ui_->processesTableView->size().width() - 120);
  ui_->processesTableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kName), QHeaderView::Stretch);
  ui_->processesTableView->verticalHeader()->setDefaultSectionSize(19);
}

}  // namespace OrbitQt