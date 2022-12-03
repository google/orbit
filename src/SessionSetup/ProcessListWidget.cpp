// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/ProcessListWidget.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QModelIndexList>
#include <QTableView>
#include <QVariant>
#include <QVector>
#include <Qt>
#include <memory>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Logging.h"
#include "SessionSetup/OverlayWidget.h"
#include "ui_ProcessListWidget.h"

namespace orbit_session_setup {

constexpr int kProcessesRowHeight = 19;

using orbit_grpc_protos::ProcessInfo;

namespace {

ProcessInfo GetProcessFromIndex(const QModelIndex& index) {
  ORBIT_CHECK(index.isValid());
  ORBIT_CHECK(index.data(Qt::UserRole).canConvert<const ProcessInfo*>());
  return *index.data(Qt::UserRole).value<const ProcessInfo*>();
}

}  // namespace

ProcessListWidget::ProcessListWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::ProcessListWidget>()) {
  ui_->setupUi(this);
  ui_->overlay->raise();

  ui_->overlay->SetCancelable(false);
  ui_->overlay->SetStatusMessage("Loading processes...");

  proxy_model_.setSourceModel(&model_);
  proxy_model_.setSortRole(Qt::EditRole);
  proxy_model_.setFilterCaseSensitivity(Qt::CaseInsensitive);

  ui_->tableView->setModel(&proxy_model_);
  ui_->tableView->setSortingEnabled(true);
  ui_->tableView->sortByColumn(static_cast<int>(ProcessItemModel::Column::kCpu),
                               Qt::DescendingOrder);
  ui_->tableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kPid), QHeaderView::ResizeToContents);
  ui_->tableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kCpu), QHeaderView::ResizeToContents);
  ui_->tableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ProcessItemModel::Column::kName), QHeaderView::Stretch);
  ui_->tableView->verticalHeader()->setDefaultSectionSize(kProcessesRowHeight);
  ui_->tableView->verticalHeader()->setVisible(false);

  QObject::connect(ui_->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this,
                   &ProcessListWidget::HandleSelectionChanged);
  QObject::connect(ui_->tableView, &QTableView::doubleClicked, this,
                   &ProcessListWidget::TryConfirm);
  QObject::connect(ui_->filterLineEdit, &QLineEdit::returnPressed, this,
                   &ProcessListWidget::TryConfirm);
  QObject::connect(ui_->filterLineEdit, &QLineEdit::textChanged, &proxy_model_,
                   &QSortFilterProxyModel::setFilterFixedString);
}

ProcessListWidget::~ProcessListWidget() noexcept = default;

void ProcessListWidget::Clear() {
  model_.Clear();
  ui_->overlay->setVisible(false);
}

std::optional<orbit_grpc_protos::ProcessInfo> ProcessListWidget::GetSelectedProcess() const {
  const QItemSelectionModel* model = ui_->tableView->selectionModel();

  if (!model->hasSelection()) {
    return std::nullopt;
  }

  ORBIT_CHECK(!model->selectedRows().empty());
  return GetProcessFromIndex(model->selectedRows().first());
}

void ProcessListWidget::HandleSelectionChanged(const QModelIndex& index) {
  if (!index.isValid()) {
    emit ProcessSelectionCleared();
    return;
  }

  emit ProcessSelected(GetProcessFromIndex(index));
}

bool ProcessListWidget::TrySelectProcessByName(std::string_view process_name) {
  QModelIndexList matches = proxy_model_.match(
      proxy_model_.index(0, static_cast<int>(ProcessItemModel::Column::kName)), Qt::DisplayRole,
      QVariant::fromValue(QString::fromUtf8(process_name.data(), process_name.size())));

  if (matches.isEmpty()) return false;

  ui_->tableView->selectionModel()->setCurrentIndex(
      matches[0], {QItemSelectionModel::SelectCurrent, QItemSelectionModel::Rows});
  return true;
}

void ProcessListWidget::UpdateList(QVector<ProcessInfo> list) {
  ui_->overlay->setVisible(false);
  bool had_processes_before = model_.HasProcesses();
  model_.SetProcesses(std::move(list));

  // In case there is a selection already, nothing changes but the `Selected` signal is emitted.
  if (std::optional<ProcessInfo> process_opt = GetSelectedProcess(); process_opt != std::nullopt) {
    emit ProcessSelected(std::move(process_opt.value()));
    return;
  }

  if (!name_to_select_.empty()) {
    bool success = TrySelectProcessByName(name_to_select_);
    if (success) {
      ORBIT_LOG("Selected remembered process with name: %s", name_to_select_);
      return;
    }
  }

  // The first time a list of processes arrives, the cpu utilization values are not valid, since
  // they are computed as an average since the last time the list was refreshed. Hence return here
  // and do not perform a selection.
  if (!had_processes_before) {
    ui_->overlay->setVisible(true);
    return;
  }

  // This selects the first (top most) row. The table is sorted by cpu usage (%) by default, so
  // unless the user changed the sorting, this will select the process with the highest cpu load
  ui_->tableView->selectRow(0);
}

void ProcessListWidget::TryConfirm() {
  if (std::optional<ProcessInfo> process_opt = GetSelectedProcess(); process_opt.has_value()) {
    emit ProcessConfirmed(std::move(process_opt.value()));
  }
}

}  // namespace orbit_session_setup