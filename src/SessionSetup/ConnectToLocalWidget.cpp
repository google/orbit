// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToLocalWidget.h"

#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <grpc/impl/codegen/connectivity_state.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QtCore>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "ClientFlags/ClientFlags.h"
#include "ClientServices/ProcessManager.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "SessionSetup/Connections.h"
#include "ui_ConnectToLocalWidget.h"

namespace orbit_session_setup {

namespace {
[[nodiscard]] std::shared_ptr<grpc::Channel> CreateLocalhostGrpcChannel() {
  const std::string target{absl::StrFormat("127.0.0.1:%d", absl::GetFlag(FLAGS_grpc_port))};
  return grpc::CreateCustomChannel(target, grpc::InsecureChannelCredentials(),
                                   grpc::ChannelArguments());
}
}  // namespace

// The destructor needs to be defined here because it needs to see the type
// `Ui::ConnectToLocalWidget`. The header file only contains a forward declaration.
ConnectToLocalWidget::~ConnectToLocalWidget() = default;

ConnectToLocalWidget::ConnectToLocalWidget(QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::ConnectToLocalWidget>()),
      local_connection_(CreateLocalhostGrpcChannel(), nullptr),
      check_connection_timer_(this) {
  ui_->setupUi(this);

  QObject::connect(ui_->radioButton, &QRadioButton::toggled, ui_->contentFrame,
                   &QWidget::setEnabled);

  QObject::connect(ui_->startOrbitServiceButton, &QPushButton::clicked, this,
                   &ConnectToLocalWidget::OnStartOrbitServiceButtonClicked);

  QObject::connect(&check_connection_timer_, &QTimer::timeout, this,
                   &ConnectToLocalWidget::CheckAndSignalConnection);
  CheckAndSignalConnection();
  check_connection_timer_.start(std::chrono::milliseconds{250});

  SetupProcessListUpdater();

  qRegisterMetaType<QVector<orbit_grpc_protos::ProcessInfo>>(
      "QVector<orbit_grpc_protos::ProcessInfo>");
}

void ConnectToLocalWidget::CheckAndSignalConnection() {
  if (local_connection_.GetGrpcChannel()->GetState(true) == GRPC_CHANNEL_READY) {
    ui_->statusLabel->setText("Connected to OrbitService");
    emit Connected();
    return;
  }
  if (local_connection_.GetOrbitServiceInstance() == nullptr) {
    ui_->statusLabel->setText("Waiting for OrbitService");
  } else {
    ui_->statusLabel->setText("Connecting to OrbitService ...");
  }
  emit Disconnected();
}

void ConnectToLocalWidget::SetOrbitServiceInstanceCreateFunction(
    OrbitServiceInstanceCreator&& creator) {
  orbit_service_instance_creator_ = std::move(creator);
  ui_->startOrbitServiceButton->setEnabled(true);
}

void ConnectToLocalWidget::OnStartOrbitServiceButtonClicked() {
  ORBIT_CHECK(orbit_service_instance_creator_ != nullptr);

  ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>> orbit_service_instance_or_error =
      orbit_service_instance_creator_();

  if (orbit_service_instance_or_error.has_error()) {
    QMessageBox::critical(
        this, "Error while starting OrbitService",
        QString::fromStdString(orbit_service_instance_or_error.error().message()));
    return;
  }

  local_connection_ = LocalConnection(CreateLocalhostGrpcChannel(),
                                      std::move(orbit_service_instance_or_error.value()));
  SetupProcessListUpdater();

  QObject::connect(local_connection_.GetOrbitServiceInstance(),
                   &OrbitServiceInstance::ErrorOccurred, this, [this](const QString& message) {
                     QMessageBox::critical(this, "OrbitService Error", message);
                     local_connection_.orbit_service_instance_ = nullptr;
                     emit Disconnected();
                   });
}

void ConnectToLocalWidget::SetConnection(LocalConnection&& connection) {
  local_connection_ = std::move(connection);
  SetupProcessListUpdater();
}

void ConnectToLocalWidget::SetupProcessListUpdater() {
  local_connection_.GetProcessManager()->SetProcessListUpdateListener(
      [self = QPointer<ConnectToLocalWidget>(this)](
          std::vector<orbit_grpc_protos::ProcessInfo> process_list) {
        if (self == nullptr) return;
        emit self->ProcessListUpdated(
            QVector<orbit_grpc_protos::ProcessInfo>(process_list.begin(), process_list.end()));
      });
}

QRadioButton* ConnectToLocalWidget::GetRadioButton() { return ui_->radioButton; }

}  // namespace orbit_session_setup