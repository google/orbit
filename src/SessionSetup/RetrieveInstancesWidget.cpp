// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/RetrieveInstancesWidget.h"

#include <QCoreApplication>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QSettings>
#include <QVariant>
#include <memory>
#include <optional>

#include "MetricsUploader/ScopedMetric.h"
#include "MetricsUploader/orbit_log_event.pb.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/PersistentStorage.h"
#include "SessionSetup/RetrieveInstances.h"
#include "ui_RetrieveInstancesWidget.h"

namespace orbit_session_setup {

using orbit_ggp::Instance;
using orbit_ggp::Project;
using LoadProjectsAndInstancesResult = RetrieveInstances::LoadProjectsAndInstancesResult;
using InstanceListScope = orbit_ggp::Client::InstanceListScope;
using orbit_metrics_uploader::OrbitLogEvent;
using orbit_metrics_uploader::ScopedMetric;

RetrieveInstancesWidget::~RetrieveInstancesWidget() = default;

RetrieveInstancesWidget::RetrieveInstancesWidget(QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::RetrieveInstancesWidget>()),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()),
      s_idle_(&state_machine_),
      s_loading_(&state_machine_),
      s_initial_loading_failed_(&state_machine_) {
  ui_->setupUi(this);

  SetupStateMachine();

  QObject::connect(ui_->filterLineEdit, &QLineEdit::textChanged, this,
                   &RetrieveInstancesWidget::FilterTextChanged);
  QObject::connect(ui_->reloadButton, &QPushButton::clicked, this,
                   &RetrieveInstancesWidget::OnReloadButtonClicked);
  QObject::connect(ui_->projectComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                   &RetrieveInstancesWidget::OnProjectComboBoxCurrentIndexChanged);
  QObject::connect(ui_->allCheckBox, &QCheckBox::clicked, this,
                   &RetrieveInstancesWidget::OnAllCheckboxClicked);
}

void RetrieveInstancesWidget::SetupStateMachine() {
  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);

  s_idle_.assignProperty(ui_->projectComboBox, "enabled", true);
  s_idle_.assignProperty(ui_->filterLineEdit, "enabled", true);
  s_idle_.assignProperty(ui_->allCheckBox, "enabled", true);
  s_idle_.assignProperty(ui_->reloadButton, "enabled", true);
  s_idle_.addTransition(this, &RetrieveInstancesWidget::LoadingStarted, &s_loading_);

  s_loading_.assignProperty(ui_->projectComboBox, "enabled", false);
  s_loading_.assignProperty(ui_->filterLineEdit, "enabled", false);
  s_loading_.assignProperty(ui_->allCheckBox, "enabled", false);
  s_loading_.assignProperty(ui_->reloadButton, "enabled", false);
  s_loading_.addTransition(this, &RetrieveInstancesWidget::LoadingSuccessful, &s_idle_);
  s_loading_.addTransition(this, &RetrieveInstancesWidget::LoadingFailed, &s_idle_);
  s_loading_.addTransition(this, &RetrieveInstancesWidget::InitialLoadingFailed,
                           &s_initial_loading_failed_);

  s_initial_loading_failed_.assignProperty(ui_->projectComboBox, "enabled", false);
  s_initial_loading_failed_.assignProperty(ui_->filterLineEdit, "enabled", false);
  s_initial_loading_failed_.assignProperty(ui_->allCheckBox, "enabled", false);
  s_initial_loading_failed_.assignProperty(ui_->reloadButton, "enabled", true);
  s_initial_loading_failed_.addTransition(this, &RetrieveInstancesWidget::LoadingStarted,
                                          &s_loading_);
}

void RetrieveInstancesWidget::Start() {
  CHECK(retrieve_instances_ != nullptr);
  state_machine_.setInitialState(&s_loading_);
  state_machine_.start();

  ui_->allCheckBox->setChecked(LoadInstancesScopeFromPersistentStorage() ==
                               InstanceListScope::kAllReservedInstances);

  InitialLoad(LoadLastSelectedProjectFromPersistentStorage());
}

InstanceListScope RetrieveInstancesWidget::GetSelectedInstanceListScope() const {
  return ui_->allCheckBox->isChecked() ? InstanceListScope::kAllReservedInstances
                                       : InstanceListScope::kOnlyOwnInstances;
}

void RetrieveInstancesWidget::InitialLoad(const std::optional<Project>& remembered_project) {
  CHECK(ui_->projectComboBox->count() == 0);
  emit LoadingStarted();
  ScopedMetric metric{metrics_uploader_, OrbitLogEvent::ORBIT_INSTANCES_INITIAL_LOAD};
  retrieve_instances_->LoadProjectsAndInstances(remembered_project, GetSelectedInstanceListScope())
      .Then(main_thread_executor_.get(),
            // The metric gets its own future continuation, so the time measured is only the
            // time that the call took.
            [metric = std::move(metric)](auto loading_result) mutable {
              if (loading_result.has_error()) metric.SetStatusCode(OrbitLogEvent::INTERNAL_ERROR);
              return loading_result;
            })
      .Then(main_thread_executor_.get(),
            [this](ErrorMessageOr<LoadProjectsAndInstancesResult> loading_result) {
              // `this` still exists when this lambda is executed. This is enforced, because
              // main_thread_executor_ is a member of `this`. When `this` is destroyed,
              // main_thread_executor_ is destroyed and the lambda is not executed. Check
              // Future::Then for details about the lambda not being executed.

              if (loading_result.has_error()) {
                emit InitialLoadingFailed();
                OnInstancesLoadingReturned(loading_result.error());
                return;
              }

              OnInitialLoadingReturnedSuccess(loading_result.value());
            });
}

void RetrieveInstancesWidget::OnInitialLoadingReturnedSuccess(
    LoadProjectsAndInstancesResult initial_load_result) {
  CHECK(ui_->projectComboBox->count() == 0);

  // From here on the projectComboBox is filled. To not trigger currentIndexChanged signals, the
  // signals of projectComboBox are blocked until the end of this function.
  ui_->projectComboBox->blockSignals(true);

  QString default_project_text =
      QString("Default Project (%1)").arg(initial_load_result.default_project.display_name);
  ui_->projectComboBox->addItem(default_project_text);

  QVector<Project>& projects{initial_load_result.projects};
  std::sort(projects.begin(), projects.end(),
            [](const Project& p1, const Project& p2) { return p1.display_name < p2.display_name; });

  for (const Project& project : initial_load_result.projects) {
    QString text = project.display_name;
    if (project == initial_load_result.default_project) {
      text.append(" (default)");
    }
    ui_->projectComboBox->addItem(text, QVariant::fromValue(project));

    // initial_load_result.instances is the list for a specific project, which is
    // initial_load_result.project_of_instances. (This can be different than the remembered project
    // which was used in InitialLoad.) Since the initial_load_result.instances will be shown by
    // ConnectToStadiaWidget, the combobox should have the project selected that belongs to this
    // list. This selection is done here.
    if (project == initial_load_result.project_of_instances) {
      // last added item
      ui_->projectComboBox->setCurrentIndex(ui_->projectComboBox->count() - 1);
    }
  }

  ui_->projectComboBox->blockSignals(false);

  OnInstancesLoadingReturned(initial_load_result.instances);
}

void RetrieveInstancesWidget::OnInstancesLoadingReturned(
    const ErrorMessageOr<QVector<Instance>>& loading_result) {
  if (loading_result.has_error()) {
    ERROR("instance loading returned with error: %s", loading_result.error().message());
    QMessageBox::critical(this, QCoreApplication::applicationName(),
                          QString::fromStdString(loading_result.error().message()));
    emit LoadingFailed();
    return;
  }
  emit LoadingSuccessful(loading_result.value());
}

void RetrieveInstancesWidget::OnReloadButtonClicked() {
  // Initial loading failed
  if (ui_->projectComboBox->count() == 0) {
    InitialLoad(std::nullopt);
    return;
  }

  std::optional<Project> selected_project;
  if (ui_->projectComboBox->currentData().canConvert<Project>()) {
    selected_project = ui_->projectComboBox->currentData().value<Project>();
  }

  emit LoadingStarted();
  retrieve_instances_->LoadInstancesWithoutCache(selected_project, GetSelectedInstanceListScope())
      .Then(main_thread_executor_.get(),
            [this](const ErrorMessageOr<QVector<Instance>>& load_result) {
              // `this` still exists when this lambda is executed. This is enforced, because
              // main_thread_executor_ is a member of `this`. When `this` is destroyed,
              // main_thread_executor_ is destroyed and the lambda is not executed. Check
              // Future::Then for details about the lambda not being executed.
              OnInstancesLoadingReturned(load_result);
            });
}

std::optional<orbit_ggp::Project> RetrieveInstancesWidget::GetSelectedProject() const {
  std::optional<Project> project;
  if (ui_->projectComboBox->currentData().canConvert<Project>()) {
    project = ui_->projectComboBox->currentData().value<Project>();
  }
  return project;
}

void RetrieveInstancesWidget::OnProjectComboBoxCurrentIndexChanged() {
  std::optional<Project> selected_project = GetSelectedProject();

  if (metrics_uploader_ != nullptr) {
    metrics_uploader_->SendLogEvent(OrbitLogEvent::ORBIT_PROJECT_CHANGED);
  }

  emit LoadingStarted();
  retrieve_instances_->LoadInstances(selected_project, GetSelectedInstanceListScope())
      .Then(main_thread_executor_.get(),
            [this, selected_project](const ErrorMessageOr<QVector<Instance>>& load_result) {
              // `this` still exists when this lambda is executed. This is enforced, because
              // main_thread_executor_ is a member of `this`. When `this` is destroyed,
              // main_thread_executor_ is destroyed and the lambda is not executed. Check
              // Future::Then for details about the lambda not being executed.

              if (load_result.has_value()) {
                SaveProjectToPersistentStorage(selected_project);
              } else {
                std::optional<Project> previously_selected =
                    LoadLastSelectedProjectFromPersistentStorage();
                // The upcoming call to setCurrentIndex is not user action, but only resetting to
                // previous values. Hence the signals are temporarily blocked.
                ui_->projectComboBox->blockSignals(true);
                if (previously_selected == std::nullopt) {
                  // If previously_selected is nullopt, that means QSettings contained either the
                  // default project or nothing (in which case selecting the default project is also
                  // correct). The first entry of the combobox (index 0) is the default project.
                  ui_->projectComboBox->setCurrentIndex(0);
                } else {
                  int index = ui_->projectComboBox->findData(
                      QVariant::fromValue(previously_selected.value()));
                  ui_->projectComboBox->setCurrentIndex(index);
                }
                ui_->projectComboBox->blockSignals(false);
              }

              OnInstancesLoadingReturned(load_result);
            });
}

void RetrieveInstancesWidget::OnAllCheckboxClicked() {
  InstanceListScope selected_scope = GetSelectedInstanceListScope();

  emit LoadingStarted();
  retrieve_instances_->LoadInstances(GetSelectedProject(), selected_scope)
      .Then(main_thread_executor_.get(),
            [widget = QPointer<RetrieveInstancesWidget>(this),
             selected_scope](const ErrorMessageOr<QVector<Instance>>& load_result) {
              if (widget == nullptr) return;

              if (load_result.has_value()) {
                SaveInstancesScopeToPersistentStorage(selected_scope);
              } else {
                // reset to the value before (value saved in PersistenStorage)
                widget->ui_->allCheckBox->setChecked(LoadInstancesScopeFromPersistentStorage() ==
                                                     InstanceListScope::kAllReservedInstances);
              }

              widget->OnInstancesLoadingReturned(load_result);
            });
}

}  // namespace orbit_session_setup