// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_PROFILING_TARGET_DIALOG_H_
#define ORBIT_QT_PROFILING_TARGET_DIALOG_H_

#include <QDialog>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStateMachine>
#include <memory>
#include <optional>
#include <variant>

#include "ConnectionArtifacts.h"
#include "MainThreadExecutor.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientServices/ProcessManager.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/InstanceItemModel.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "ProcessItemModel.h"
#include "absl/container/flat_hash_map.h"
#include "deploymentconfigurations.h"
#include "grpcpp/channel.h"
#include "servicedeploymanager.h"
#include "ui_ProfilingTargetDialog.h"

namespace OrbitQt {

class ProfilingTargetDialog : public QDialog {
  Q_OBJECT
 public:
  // TODO(antonrohr) maybe remove grpc port
  explicit ProfilingTargetDialog(ConnectionArtifacts* connection_artifacts,
                                 MainThreadExecutor* main_thread_executer,
                                 QWidget* parent = nullptr);
  std::variant<std::monostate, const ConnectionArtifacts*, QString> Exec();

 private slots:
  void on_refreshButton_clicked() { ReloadInstances(); }
  void on_loadCaptureRadioButton_toggled(bool checked);
  void on_connectToStadiaInstanceRadioButton_toggled(bool checked);
  void on_connectButton_clicked() { ConnectToInstance(); }
  void on_instancesTableView_doubleClicked() { ConnectToInstance(); }
  void on_processesTableView_doubleClicked() { on_selectProcessButton_clicked(); }
  void on_selectProcessButton_clicked();
  void on_loadFromFileButton_clicked();
  void on_rememberCheckBox_toggled(bool checked);
  void on_accepted() { LOG("accepted"); }

 private:
  // Qt Ui member
  std::unique_ptr<Ui::ProfilingTargetDialog> ui_;
  OrbitGgp::InstanceItemModel instance_model_;
  ProcessItemModel process_model_;
  QSortFilterProxyModel process_proxy_model_;
  bool connect_clicked_ = false;

  ConnectionArtifacts* connection_artifacts_;

  QPointer<OrbitGgp::Client> ggp_client_;
  absl::flat_hash_map<std::string, OrbitSsh::Credentials> instance_credentials_;
  MainThreadExecutor* main_thread_executer_;
  std::variant<std::monostate, const ConnectionArtifacts*, QString> dialog_result_;
  QSettings settings_;
  QString file_to_load_;

  void SetupUi();
  void ConnectToInstance();
  void DisconnectFromInstance();
  void DeployOrbitService();
  void ReloadInstances();
  void DisplayErrorToUser(const QString& message);
  void EnableConfirm(const QString& text);
  void ResizeTables();
};

}  // namespace OrbitQt

#endif  // ORBIT_QT_PROFILING_TARGET_DIALOG_H_