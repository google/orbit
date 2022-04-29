// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_PROCESS_LAUNCHER_WIDGET_H_
#define SESSION_SETUP_PROCESS_LAUNCHER_WIDGET_H_

#include <QPushButton>
#include <QWidget>

#include "ClientServices/ProcessManager.h"

namespace Ui {
class ProcessLauncherWidget;
}

namespace orbit_session_setup {

class ProcessLauncherWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ProcessLauncherWidget(QWidget* parent = 0);
  ~ProcessLauncherWidget();

  void SetProcessManager(orbit_client_services::ProcessManager* process_manager) {
    process_manager_ = process_manager;
  }

  [[nodiscard]] QPushButton* GetLaunchButton();

  struct ProcessToLaunch {
    std::string executable_path;
    std::string working_directory;
    std::string arguments;
    bool spin_on_entry_point;
  };

  const ErrorMessageOr<orbit_grpc_protos::ProcessInfo>& GetLastLaunchedProcessOrError() const {
    return last_launched_process_or_error_;
  }

 private slots:
  void on_BrowseProcessButton_clicked();
  void on_BrowseWorkingDirButton_clicked();
  void on_LaunchButton_clicked();

 private:
  Ui::ProcessLauncherWidget* ui;

  ErrorMessageOr<orbit_grpc_protos::ProcessInfo> last_launched_process_or_error_;
  orbit_client_services::ProcessManager* process_manager_ = nullptr;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_PROCESS_LAUNCHER_WIDGET_H_
