// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_PROFILING_TARGET_DIALOG_H_
#define ORBIT_QT_PROFILING_TARGET_DIALOG_H_

#include <QDialog>
#include <QHistoryState>
#include <QModelIndex>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QState>
#include <QStateMachine>
#include <QString>
#include <QWidget>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "Connections.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientServices/ProcessManager.h"
#include "ProcessItemModel.h"
#include "TargetConfiguration.h"
#include "grpcpp/channel.h"
#include "process.pb.h"

namespace Ui {
class ProfilingTargetDialog;  // IWYU pragma: keep
}
namespace orbit_qt {

class ProfilingTargetDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ProfilingTargetDialog(SshConnectionArtifacts* ssh_connection_artifacts,
                                 std::optional<TargetConfiguration> target_configuration_opt,
                                 orbit_metrics_uploader::MetricsUploader* metrics_uploader,
                                 QWidget* parent = nullptr);
  ~ProfilingTargetDialog() noexcept override;

  [[nodiscard]] std::optional<TargetConfiguration> Exec();
 private slots:
  void SelectFile();
  void SetupStadiaProcessManager();
  void SetupLocalProcessManager();
  void TearDownProcessManager();
  void ProcessSelectionChanged(const QModelIndex& current);
  void ConnectToLocal();

 signals:
  void FileSelected();
  void ProcessSelected();
  void NoProcessSelected();
  void StadiaIsConnected();
  void ProcessListUpdated();

 private:
  std::unique_ptr<Ui::ProfilingTargetDialog> ui_;

  ProcessItemModel process_model_;
  QSortFilterProxyModel process_proxy_model_;

  std::unique_ptr<ProcessData> process_;
  std::unique_ptr<ProcessManager> process_manager_;

  std::shared_ptr<grpc::Channel> local_grpc_channel_;
  uint16_t local_grpc_port_;

  std::filesystem::path selected_file_path_;

  orbit_metrics_uploader::MetricsUploader* metrics_uploader_;

  // State Machine & States
  QStateMachine state_machine_;
  QState state_stadia_;
  QHistoryState state_stadia_history_;
  QState state_stadia_connecting_;
  QState state_stadia_connected_;
  QState state_stadia_processes_loading_;
  QState state_stadia_process_selected_;
  QState state_stadia_no_process_selected_;

  QState state_file_;
  QHistoryState state_file_history_;
  QState state_file_selected_;
  QState state_file_no_selection_;

  QState state_local_;
  QHistoryState state_local_history_;
  QState state_local_connecting_;
  QState state_local_connected_;
  QState state_local_processes_loading_;
  QState state_local_process_selected_;
  QState state_local_no_process_selected_;

  void SetupStadiaStates();
  void SetupFileStates();
  void SetupLocalStates();
  void SetStateMachineInitialStateFromTarget(TargetConfiguration config);
  void SetStateMachineInitialState();
  [[nodiscard]] bool TrySelectProcess(const std::string& process);
  void OnProcessListUpdate(std::vector<orbit_grpc_protos::ProcessInfo> process_list);
  void SetupProcessManager(const std::shared_ptr<grpc::Channel>& grpc_channel);
  void SetTargetAndStateMachineInitialState(StadiaTarget target);
  void SetTargetAndStateMachineInitialState(LocalTarget target);
  void SetTargetAndStateMachineInitialState(FileTarget target);
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_PROFILING_TARGET_DIALOG_H_