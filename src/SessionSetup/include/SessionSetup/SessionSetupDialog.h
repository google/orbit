// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_SESSION_SETUP_DIALOG_H_
#define SESSION_SETUP_SESSION_SETUP_DIALOG_H_

#include <grpcpp/channel.h>

#include <QButtonGroup>
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
#include <string>
#include <vector>

#include "Connections.h"
#include "GrpcProtos/process.pb.h"
#include "TargetConfiguration.h"

namespace Ui {
class SessionSetupDialog;  // IWYU pragma: keep
}
namespace orbit_session_setup {

class SessionSetupDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SessionSetupDialog(SshConnectionArtifacts* ssh_connection_artifacts,
                              std::optional<TargetConfiguration> target_configuration_opt,
                              QWidget* parent = nullptr);
  ~SessionSetupDialog() override;

  [[nodiscard]] std::optional<TargetConfiguration> Exec();
 private slots:
  void ConnectLocalAndProcessWidget();
  void DisconnectLocalAndProcessWidget();

 signals:
  void ProcessSelected();
  void NoProcessSelected();
  void ProcessListUpdated();

 private:
  std::unique_ptr<Ui::SessionSetupDialog> ui_;
  QButtonGroup button_group_;

  std::filesystem::path selected_file_path_;

  // State Machine & States
  QStateMachine state_machine_;

  QState state_file_;
  QHistoryState state_file_history_;
  QState state_file_selected_;
  QState state_file_no_selection_;

  QState state_local_;
  QHistoryState state_local_history_;
  QState state_local_connecting_;
  QState state_local_connected_;
  QState state_local_no_process_selected_;
  QState state_local_process_selected_;

  void SetupFileStates();
  void SetupLocalStates();
  void SetTargetAndStateMachineInitialState(SshTarget target);
  void SetTargetAndStateMachineInitialState(LocalTarget target);
  void SetTargetAndStateMachineInitialState(const FileTarget& target);
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_SESSION_SETUP_DIALOG_H_