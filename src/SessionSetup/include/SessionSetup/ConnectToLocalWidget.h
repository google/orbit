// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_CONNECT_TO_LOCAL_WIDGET_H_
#define SESSION_SETUP_CONNECT_TO_LOCAL_WIDGET_H_

#include <grpcpp/channel.h>

#include <QObject>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <functional>
#include <memory>
#include <utility>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/OrbitServiceInstance.h"

namespace Ui {
class ConnectToLocalWidget;
}

namespace orbit_session_setup {

using OrbitServiceInstanceCreator =
    std::function<ErrorMessageOr<std::unique_ptr<OrbitServiceInstance>>()>;

// ConnectToLocalWidget provides a UI to connect to a running OrbitService and start an OrbitService
// instance. ConnectToLocalWidget sets up a gRPC channel to localhost with port `--grpc_port` and
// checks every 250ms whether this channel is connected to OrbitService. If the user clicks the
// "Start OrbitService" button, a OrbitServiceInstance is set up, to which the gRPC channel can
// connect to.
class ConnectToLocalWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool active READ IsActive WRITE SetActive)

 public:
  explicit ConnectToLocalWidget(QWidget* parent = nullptr);
  ~ConnectToLocalWidget() override;

  [[nodiscard]] bool IsActive() const;

  void SetOrbitServiceInstanceCreateFunction(OrbitServiceInstanceCreator&& creator);

  void SetConnection(LocalConnection&& connection) { local_connection_ = std::move(connection); }
  [[nodiscard]] LocalConnection&& TakeConnection() { return std::move(local_connection_); }

  [[nodiscard]] const std::shared_ptr<grpc::Channel>& GetGrpcChannel() const {
    return local_connection_.GetGrpcChannel();
  }

 public slots:
  void SetActive(bool value);

 signals:
  void Activated();
  void Connected();
  void Disconnected();

 private:
  std::unique_ptr<Ui::ConnectToLocalWidget> ui_;
  OrbitServiceInstanceCreator orbit_service_instance_creator_ = nullptr;
  LocalConnection local_connection_;
  QTimer check_connection_timer_;

  void OnStartOrbitServiceButtonClicked();
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_CONNECT_TO_LOCAL_WIDGET_H_