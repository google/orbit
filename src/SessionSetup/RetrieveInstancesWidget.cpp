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

RetrieveInstancesWidget::~RetrieveInstancesWidget() = default;

RetrieveInstancesWidget::RetrieveInstancesWidget(MainThreadExecutor* main_thread_executor,
                                                 RetrieveInstances* retreive_instances,
                                                 QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::RetrieveInstancesWidget>()),
      main_thread_executor_(main_thread_executor),
      retreive_instances_(retreive_instances),
      s_idle_(&state_machine_),
      s_loading_(&state_machine_),
      s_initial_loading_failed_(&state_machine_) {
  CHECK(main_thread_executor != nullptr);
  CHECK(retreive_instances != nullptr);

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

std::optional<Project> RetrieveInstancesWidget::GetQSettingsProject() {
  QSettings settings;

  std::optional<Project> remembered_project;
  if (settings.contains(kSelectedProjectIdKey)) {
    remembered_project = Project{
        settings.value(kSelectedProjectDisplayNameKey).toString() /* display_name */,
        settings.value(kSelectedProjectIdKey).toString() /* id */
    };
  }
  return remembered_project;
}

void RetrieveInstancesWidget::Start() {
  state_machine_.setInitialState(&s_loading_);
  state_machine_.start();

  QSettings settings;
  ui_->allCheckBox->setChecked(settings.value(kAllInstancesKey, false).toBool());

  InitialLoad(GetQSettingsProject());
}

Client::InstanceListScope RetrieveInstancesWidget::GetInstanceListScope() {
  return ui_->allCheckBox->isChecked() ? Client::InstanceListScope::kAllReservedInstances
                                       : Client::InstanceListScope::kOnlyOwnInstances;
}

void RetrieveInstancesWidget::InitialLoad(const std::optional<Project>& remembered_project) {
  emit LoadingStarted();
  retreive_instances_->LoadProjectsAndInstances(remembered_project, GetInstanceListScope())
      .Then(main_thread_executor_,
            [widget = QPointer<RetrieveInstancesWidget>(this)](
                ErrorMessageOr<RetrieveInstances::LoadProjectsAndInstancesResult> loading_result) {
              if (widget == nullptr) return;

              if (loading_result.has_error()) {
                emit widget->InitialLoadingFailed();
                widget->OnInstancesLoadingReturned(loading_result.error());
                return;
              }

              CHECK(widget->ui_->projectComboBox->count() == 0);

              RetrieveInstances::LoadProjectsAndInstancesResult& data{loading_result.value()};

              QString default_project_text =
                  QString("Default Project (%1)").arg(data.default_project.display_name);
              widget->ui_->projectComboBox->addItem(default_project_text);

              QVector<Project>& projects{data.projects};
              std::sort(projects.begin(), projects.end(), [](const Project& p1, const Project& p2) {
                return p1.display_name.toLower() < p2.display_name.toLower();
              });

              for (const Project& project : data.projects) {
                QString text = project.display_name;
                if (project == data.default_project) {
                  text.append(" (default)");
                }
                widget->ui_->projectComboBox->addItem(text, QVariant::fromValue(project));

                if (project == data.project_of_instances) {
                  // last added item
                  widget->ui_->projectComboBox->setCurrentIndex(
                      widget->ui_->projectComboBox->count() - 1);
                }
              }

              widget->OnInstancesLoadingReturned(data.instances);
            });
}

void RetrieveInstancesWidget::OnInstancesLoadingReturned(
    const ErrorMessageOr<QVector<Instance>>& loading_result) {
  if (loading_result.has_error()) {
    QMessageBox::critical(this, QCoreApplication::applicationName(),
                          QString::fromStdString(loading_result.error().message()));
    emit LoadingFailed();
    return;
  }
  emit LoadingSuccessful(loading_result.value());
}

}  // namespace orbit_session_setup