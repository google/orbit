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

#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/RetrieveInstances.h"
#include "ui_RetrieveInstancesWidget.h"

namespace {
constexpr const char* kAllInstancesKey{"kAllInstancesKey"};
constexpr const char* kSelectedProjectIdKey{"kSelectedProjectIdKey"};
constexpr const char* kSelectedProjectDisplayNameKey{"kSelectedProjectDisplayNameKey"};
}  // namespace

namespace orbit_session_setup {

using orbit_ggp::Client;
using orbit_ggp::Instance;
using orbit_ggp::Project;
using LoadProjectsAndInstancesResult = RetrieveInstances::LoadProjectsAndInstancesResult;

namespace {

std::optional<Project> GetQSettingsProject() {
  QSettings settings;

  std::optional<Project> project;
  if (settings.contains(kSelectedProjectIdKey)) {
    project = Project{
        settings.value(kSelectedProjectDisplayNameKey).toString() /* display_name */,
        settings.value(kSelectedProjectIdKey).toString() /* id */
    };
  }
  return project;
}

}  // namespace

RetrieveInstancesWidget::~RetrieveInstancesWidget() = default;

RetrieveInstancesWidget::RetrieveInstancesWidget(RetrieveInstances* retrieve_instances,
                                                 QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::RetrieveInstancesWidget>()),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()),
      retrieve_instances_(retrieve_instances),
      s_idle_(&state_machine_),
      s_loading_(&state_machine_),
      s_initial_loading_failed_(&state_machine_) {
  CHECK(retrieve_instances != nullptr);

  ui_->setupUi(this);

  SetupStateMachine();

  QObject::connect(ui_->filterLineEdit, &QLineEdit::textChanged, this,
                   &RetrieveInstancesWidget::FilterTextChanged);
}

void RetrieveInstancesWidget::SetupStateMachine() {
  state_machine_.setGlobalRestorePolicy(QStateMachine::RestoreProperties);

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
  s_initial_loading_failed_.addTransition(this, &RetrieveInstancesWidget::LoadingStarted,
                                          &s_loading_);
}

void RetrieveInstancesWidget::Start() {
  state_machine_.setInitialState(&s_loading_);
  state_machine_.start();

  QSettings settings;
  ui_->allCheckBox->setChecked(settings.value(kAllInstancesKey, false).toBool());

  InitialLoad(GetQSettingsProject());
}

Client::InstanceListScope RetrieveInstancesWidget::GetInstanceListScope() const {
  return ui_->allCheckBox->isChecked() ? Client::InstanceListScope::kAllReservedInstances
                                       : Client::InstanceListScope::kOnlyOwnInstances;
}

void RetrieveInstancesWidget::InitialLoad(const std::optional<Project>& remembered_project) {
  emit LoadingStarted();
  retrieve_instances_->LoadProjectsAndInstances(remembered_project, GetInstanceListScope())
      .Then(main_thread_executor_.get(),
            [widget = QPointer<RetrieveInstancesWidget>(this)](
                ErrorMessageOr<LoadProjectsAndInstancesResult> loading_result) {
              if (widget == nullptr) return;

              if (loading_result.has_error()) {
                emit widget->InitialLoadingFailed();
                widget->OnInstancesLoadingReturned(loading_result.error());
                return;
              }

              widget->OnInitialLoadingReturnedSuccess(loading_result.value());
            });
}

void RetrieveInstancesWidget::OnInitialLoadingReturnedSuccess(
    LoadProjectsAndInstancesResult initial_load_result) {
  CHECK(ui_->projectComboBox->count() == 0);

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

    // initial_load_result.instance, is the list for a specific project, which is
    // initial_load_result.project_of_instances. (This can be different than the remembered project
    // which was used in InitialLoad.) Since the initial_load_result.instances will be shown by
    // ConnectToStadiaWidget, the combobox should have the project selected that belongs to this
    // list. This selection is done here.
    if (project == initial_load_result.project_of_instances) {
      // last added item
      ui_->projectComboBox->setCurrentIndex(ui_->projectComboBox->count() - 1);
    }
  }

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

}  // namespace orbit_session_setup