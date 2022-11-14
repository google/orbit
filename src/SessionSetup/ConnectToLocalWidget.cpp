// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ConnectToLocalWidget.h"

#include <absl/flags/flag.h>
#include <absl/time/time.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>

#include <QMessageBox>
#include <QPushButton>
#include <algorithm>
#include <memory>

#include "ClientFlags/ClientFlags.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "SessionSetup/Connections.h"
#include "ui_ConnectToLocalWidget.h"

namespace orbit_session_setup {

// The destructor needs to be defined here because it needs to see the type
// `Ui::ConnectToLocalWidget`. The header file only contains a forward declaration.
ConnectToLocalWidget::~ConnectToLocalWidget() = default;

ConnectToLocalWidget::ConnectToLocalWidget(QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::ConnectToLocalWidget>()),
      local_connection_(
          grpc::CreateCustomChannel(absl::StrFormat("127.0.0.1:%d", absl::GetFlag(FLAGS_grpc_port)),
                                    grpc::InsecureChannelCredentials(), grpc::ChannelArguments()),
          nullptr),
      check_connection_timer_(this) {
  ui_->setupUi(this);

  // The following is to make the radioButton behave as if it was part of an exclusive button group
  // in the parent widget (SessionSetupDialog). If a user clicks on the radioButton and it was
  // not checked before, it is checked afterwards and this widget sends the activation signal.
  // SessionSetupDialog reacts to the signal and deactivates the other widgets belonging to that
  // button group. If a user clicks on a radio button that is already checked, nothing happens, the
  // button does not get unchecked.
  QObject::connect(ui_->radioButton, &QRadioButton::clicked, this, [this](bool checked) {
    if (checked) {
      emit Activated();
    } else {
      ui_->radioButton->setChecked(true);
    }
  });

  QObject::connect(ui_->startOrbitServiceButton, &QPushButton::clicked, this,
                   &ConnectToLocalWidget::OnStartOrbitServiceButtonClicked);

  QObject::connect(&check_connection_timer_, &QTimer::timeout, this, [this]() {
    if (local_connection_.GetGrpcChannel()->GetState(true) == GRPC_CHANNEL_READY) {
      ui_->statusLabel->setText("Connected to OrbitService");
      emit Connected();
    } else {
      if (local_connection_.GetOrbitServiceInstance() == nullptr) {
        ui_->statusLabel->setText("Waiting for OrbitService");
      } else {
        ui_->statusLabel->setText("Connecting... to OrbitService");
      }
      emit Disconnected();
    }
  });
  check_connection_timer_.start(250);
}

bool ConnectToLocalWidget::IsActive() const { return ui_->contentFrame->isEnabled(); }

void ConnectToLocalWidget::SetActive(bool value) {
  ui_->contentFrame->setEnabled(value);
  ui_->radioButton->setChecked(value);
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

  local_connection_.orbit_service_instance_ = std::move(orbit_service_instance_or_error.value());

  QObject::connect(local_connection_.GetOrbitServiceInstance(),
                   &OrbitServiceInstance::ErrorOccurred, this, [this](const QString& message) {
                     QMessageBox::critical(this, "OrbitService Error", message);
                     local_connection_.orbit_service_instance_ = nullptr;
                     emit Disconnected();
                   });
}

}  // namespace orbit_session_setup