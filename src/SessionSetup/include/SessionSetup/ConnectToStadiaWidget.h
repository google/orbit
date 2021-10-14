// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_CONNECT_TO_STADIA_WIDGET_H_
#define SESSION_SETUP_CONNECT_TO_STADIA_WIDGET_H_

#include <absl/container/flat_hash_map.h>
#include <grpcpp/channel.h>

#include <QFrame>
#include <QModelIndex>
#include <QObject>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QState>
#include <QStateMachine>
#include <QString>
#include <QVector>
#include <QWidget>
#include <memory>
#include <optional>
#include <string>

#include "Connections.h"
#include "OrbitBase/Executor.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/InstanceItemModel.h"
#include "OrbitGgp/Project.h"
#include "OrbitGgp/SshInfo.h"
#include "OrbitSsh/Credentials.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/ServiceDeployManager.h"

namespace Ui {
class ConnectToStadiaWidget;
}

namespace orbit_session_setup {

class ConnectToStadiaWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool active READ IsActive WRITE SetActive)

 public:
  explicit ConnectToStadiaWidget(QWidget* parent = nullptr);
  ~ConnectToStadiaWidget() override;
  [[nodiscard]] std::optional<StadiaConnection> StopAndClearConnection();
  [[nodiscard]] bool IsActive() const;
  [[nodiscard]] const std::shared_ptr<grpc::Channel>& GetGrpcChannel() { return grpc_channel_; }
  void SetSshConnectionArtifacts(SshConnectionArtifacts* ssh_connection_artifacts);
  void ClearSshConnectionArtifacts() { ssh_connection_artifacts_ = nullptr; }
  void SetConnection(StadiaConnection connection);
  void Start();
  [[nodiscard]] std::optional<orbit_ggp::Instance> GetSelectedInstance() const {
    return selected_instance_;
  }

 public slots:
  void SetActive(bool value);

 private slots:
  void ReloadInstances();
  void CheckCredentialsAvailableOrLoad();
  void DeployOrbitService();
  void Disconnect();
  void OnConnectToStadiaRadioButtonClicked(bool checked);
  void OnErrorOccurred(const QString& message);
  void OnSelectionChanged(const QModelIndex& current);
  void UpdateRememberInstance(bool value);
  void ProjectComboBoxActivated(int index);

 signals:
  void ErrorOccurred(const QString& message);
  void ReceivedInstances();
  void InstanceSelected();
  void ReceivedSshInfo();
  void ReadyToDeploy();
  void Connect();
  void Activated();
  void Connected();
  void Disconnected();
  void InstanceReloadRequested();

 private:
  void showEvent(QShowEvent* event) override;
  std::unique_ptr<Ui::ConnectToStadiaWidget> ui_;

  orbit_ggp::InstanceItemModel instance_model_;
  QSortFilterProxyModel instance_proxy_model_;

  SshConnectionArtifacts* ssh_connection_artifacts_ = nullptr;
  std::optional<orbit_ggp::Instance> selected_instance_;
  std::unique_ptr<ServiceDeployManager> service_deploy_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;
  std::unique_ptr<orbit_ggp::Client> ggp_client_;
  std::optional<QString> remembered_instance_id_;
  QVector<orbit_ggp::Project> projects_;
  std::optional<orbit_ggp::Project> selected_project_;
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> main_thread_executor_;

  // State Machine & States
  QStateMachine state_machine_;
  QState s_idle_;
  QState s_instances_loading_;
  QState s_instance_selected_;
  QState s_waiting_for_creds_;
  QState s_deploying_;
  QState s_connected_;

  absl::flat_hash_set<std::string> instance_credentials_loading_;
  absl::flat_hash_map<std::string, ErrorMessageOr<orbit_ssh::Credentials>> instance_credentials_;

  void DetachRadioButton();
  void SetupStateMachine();
  void OnInstancesLoaded(ErrorMessageOr<QVector<orbit_ggp::Instance>> instances);
  void OnProjectsLoaded(ErrorMessageOr<QVector<orbit_ggp::Project>> projects);
  void OnSshInfoLoaded(ErrorMessageOr<orbit_ggp::SshInfo> ssh_info_result, std::string instance_id);
  void TrySelectRememberedInstance();
  void SetProject(const std::optional<orbit_ggp::Project>& project);
  void SetupProjectSelectionFlagContent();
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_CONNECT_TO_STADIA_WIDGET_H_