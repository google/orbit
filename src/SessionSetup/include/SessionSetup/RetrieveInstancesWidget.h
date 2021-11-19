// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_RETRIEVE_INSTANCES_WIDGET_H_
#define SESSION_SETUP_RETRIEVE_INSTANCES_WIDGET_H_

#include <QObject>
#include <QState>
#include <QStateMachine>
#include <QWidget>
#include <memory>

#include "MainThreadExecutor.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Project.h"
#include "SessionSetup/RetrieveInstances.h"

namespace Ui {
class RetrieveInstancesWidget;
}

namespace orbit_session_setup {

class RetrieveInstancesWidget : public QWidget {
  Q_OBJECT

 public:
  explicit RetrieveInstancesWidget(QWidget* parent = nullptr);
  ~RetrieveInstancesWidget() override;

  void Start();
  void SetRetrieveInstances(RetrieveInstances* retrieve_instances) {
    retrieve_instances_ = retrieve_instances;
  }
  void SetMetricsUploader(orbit_metrics_uploader::MetricsUploader* metrics_uploader) {
    metrics_uploader_ = metrics_uploader;
    CHECK(retrieve_instances_ != nullptr);
    retrieve_instances_->SetMetricsUploader(metrics_uploader);
  }

 signals:
  void LoadingStarted();
  void LoadingSuccessful(QVector<orbit_ggp::Instance>);
  void LoadingFailed();
  void InitialLoadingFailed();
  void FilterTextChanged(const QString& text);

 private:
  void SetupStateMachine();
  [[nodiscard]] orbit_ggp::Client::InstanceListScope GetSelectedInstanceListScope() const;
  [[nodiscard]] std::optional<orbit_ggp::Project> GetSelectedProject() const;
  void InitialLoad(const std::optional<orbit_ggp::Project>& remembered_project);
  void OnInstancesLoadingReturned(
      const ErrorMessageOr<QVector<orbit_ggp::Instance>>& loading_result);
  void OnInitialLoadingReturnedSuccess(
      RetrieveInstances::LoadProjectsAndInstancesResult initial_load_result);
  void OnReloadButtonClicked();
  void OnProjectComboBoxCurrentIndexChanged();
  void OnAllCheckboxClicked();

  std::unique_ptr<Ui::RetrieveInstancesWidget> ui_;
  std::shared_ptr<MainThreadExecutor> main_thread_executor_;
  RetrieveInstances* retrieve_instances_;
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_ = nullptr;
  QStateMachine state_machine_;
  QState s_idle_;
  QState s_loading_;
  QState s_initial_loading_failed_;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_RETRIEVE_INSTANCES_WIDGET_H_