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
#include "OrbitGgp/Client.h"
#include "OrbitGgp/SshInfo.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "TargetConfiguration.h"

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

  struct ResultData {
    QString error_message_ = "";
    std::optional<orbit_ggp::Instance> maybe_instance_ = std::nullopt;
    std::unique_ptr<orbit_client_data::ProcessData> process_data_ = nullptr;
    std::unique_ptr<orbit_session_setup::ServiceDeployManager> service_deploy_manager_ = nullptr;
    std::shared_ptr<grpc_impl::Channel> grpc_channel_ = nullptr;
  };

  ResultData result_data_;

  std::unique_ptr<orbit_ggp::Client> ggp_client_;
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> main_thread_executor_;

  [[nodiscard]] StadiaTarget CreateTargetFromResult();

  [[nodiscard]] bool StartConnection();
  [[nodiscard]] std::optional<ServiceDeployManager::GrpcPort> DeployOrbitService(
      orbit_ggp::SshInfo& ssh_info);
  void ListProcessesAndSaveProcess(std::shared_ptr<grpc_impl::Channel> grpc_channel);
  void LoadInstanceDetailsAsync();
  void OnSshInfoAvailable(ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result);

  void CloseDialogIfResultIsReady();
  void SetStatusMessage(const QString& message);
};

}  // namespace orbit_session_setup

#endif