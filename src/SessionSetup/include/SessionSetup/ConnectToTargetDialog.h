// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_CONNECT_TO_TARGET_DIALOG_H_
#define SESSION_SETUP_CONNECT_TO_TARGET_DIALOG_H_

#include <QDialog>
#include <QString>
#include <optional>

#include "Connections.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/SshInfo.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "TargetConfiguration.h"
#include "grpcpp/channel.h"

namespace Ui {
class ConnectToTargetDialog;  // IWYU pragma: keep
}
namespace orbit_session_setup {

// Simple dialog to show progress while connecting to a specified instance id and process id.
// This takes care of establishing the connection and deploying Orbit service, and will either
// return a TargetConfiguration or nullopt on Exec. If a nullopt is returned, an error was
// displayed to the user and has been logged.
class ConnectToTargetDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ConnectToTargetDialog(SshConnectionArtifacts* ssh_connection_artifacts,
                                 const QString& instance_id, uint32_t process_id,
                                 orbit_metrics_uploader::MetricsUploader* metrics_uploader,
                                 QWidget* parent = nullptr);
  ~ConnectToTargetDialog() override;

  [[nodiscard]] std::optional<TargetConfiguration> Exec();

 private:
  std::unique_ptr<Ui::ConnectToTargetDialog> ui_;

  SshConnectionArtifacts* ssh_connection_artifacts_;

  QString instance_id_;
  uint32_t process_id_;
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_;

  struct ConnectionData {
    std::unique_ptr<orbit_client_data::ProcessData> process_data_;
    std::unique_ptr<orbit_session_setup::ServiceDeployManager> service_deploy_manager_;
    std::shared_ptr<grpc::Channel> grpc_channel_;
  };

  using MaybeSshAndInstanceData =
      std::tuple<ErrorMessageOr<orbit_ggp::SshInfo>, ErrorMessageOr<orbit_ggp::Instance>>;

  std::unique_ptr<orbit_ggp::Client> ggp_client_;
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> main_thread_executor_;

  ErrorMessageOr<StadiaTarget> OnAsyncDataAvailable(MaybeSshAndInstanceData ssh_instance_data);

  [[nodiscard]] StadiaTarget CreateTarget(ConnectionData result,
                                          orbit_ggp::Instance instance) const;
  [[nodiscard]] ErrorMessageOr<orbit_session_setup::ServiceDeployManager::GrpcPort>
  DeployOrbitService(orbit_session_setup::ServiceDeployManager* service_deploy_manager);
  [[nodiscard]] ErrorMessageOr<std::unique_ptr<orbit_client_data::ProcessData>>
  FindSpecifiedProcess(std::shared_ptr<grpc::Channel> grpc_channel, uint32_t process_id);

  void SetStatusMessage(const QString& message);
  void LogAndDisplayError(const ErrorMessage& message);
};

}  // namespace orbit_session_setup

#endif