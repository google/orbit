// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_PROCESS_LAUNCHER_WIDGET_H_
#define SESSION_SETUP_PROCESS_LAUNCHER_WIDGET_H_

#include <QObject>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <memory>

#include "ClientServices/ProcessManager.h"
#include "GrpcProtos/process.pb.h"

namespace Ui {
class ProcessLauncherWidget;
}

namespace orbit_session_setup {

class ProcessLauncherWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ProcessLauncherWidget(QWidget* parent = nullptr);
  ~ProcessLauncherWidget() override;
  void SetProcessManager(orbit_client_services::ProcessManager* process_manager);

 signals:
  void ProcessLaunched(const orbit_grpc_protos::ProcessInfo& process_info);

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 private slots:
  void on_BrowseProcessButton_clicked();
  void on_BrowseWorkingDirButton_clicked();
  void on_LaunchButton_clicked();

  // TODO(https://github.com/google/orbit/issues/4589): Remove redundant "private" once slots is not
  // needed anymore above.
 private:  // NOLINT(readability-redundant-access-specifiers)
  std::unique_ptr<Ui::ProcessLauncherWidget> ui_;
  orbit_client_services::ProcessManager* process_manager_ = nullptr;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_PROCESS_LAUNCHER_WIDGET_H_
