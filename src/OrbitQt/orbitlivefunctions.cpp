// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/orbitlivefunctions.h"

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/meta/type_traits.h>
#include <stddef.h>

#include <QBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLayout>
#include <QModelIndex>
#include <QObject>
#include <QTabWidget>
#include <utility>

#include "ClientData/ScopeId.h"
#include "DataViews/DataView.h"
#include "DataViews/LiveFunctionsDataView.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitQt/HistogramWidget.h"
#include "OrbitQt/orbitdataviewpanel.h"
#include "OrbitQt/orbittablemodel.h"
#include "OrbitQt/orbittreeview.h"
#include "ui_orbitlivefunctions.h"

using orbit_client_data::FunctionInfo;

OrbitLiveFunctions::OrbitLiveFunctions(QWidget* parent)
    : QWidget(parent), ui_(new Ui::OrbitLiveFunctions) {
  ui_->setupUi(this);
}

OrbitLiveFunctions::~OrbitLiveFunctions() { delete ui_; }

void OrbitLiveFunctions::Initialize(OrbitApp* app, SelectionType selection_type,
                                    FontType font_type) {
  live_functions_.emplace(app);
  orbit_data_views::DataView* data_view = &live_functions_->GetDataView();
  ui_->data_view_panel_->Initialize(data_view, selection_type, font_type);

  live_functions_->SetAddIteratorCallback(
      [this](uint64_t id, const FunctionInfo* function) { this->AddIterator(id, function); });

  all_events_iterator_ = new OrbitEventIterator(this);
  all_events_iterator_->SetNextButtonCallback([this]() {
    if (!this->live_functions_->OnAllNextButton()) {
      return;
    }
    for (auto& iterator_ui : iterator_uis_) {
      uint64_t index = iterator_ui.first;
      iterator_ui.second->SetCurrentTime(this->live_functions_->GetStartTime(index));
    }
  });
  all_events_iterator_->SetPreviousButtonCallback([this]() {
    if (!this->live_functions_->OnAllPreviousButton()) {
      return;
    }
    for (auto& iterator_ui : iterator_uis_) {
      uint64_t index = iterator_ui.first;
      iterator_ui.second->SetCurrentTime(this->live_functions_->GetStartTime(index));
    }
  });
  all_events_iterator_->SetFunctionName("All functions");
  all_events_iterator_->HideDeleteButton();
  all_events_iterator_->DisableButtons();
  dynamic_cast<QBoxLayout*>(ui_->iteratorFrame->layout())
      ->insertWidget(ui_->iteratorFrame->layout()->count() - 1, all_events_iterator_);

  QObject::connect(ui_->histogram_widget_, &orbit_qt::HistogramWidget::SignalSelectionRangeChange,
                   this, [this](std::optional<orbit_statistics::HistogramSelectionRange> range) {
                     emit SignalSelectionRangeChange(range);
                   });

  ui_->histogram_title_->setText(ui_->histogram_widget_->GetTitle());
  QObject::connect(ui_->histogram_widget_, &orbit_qt::HistogramWidget::SignalTitleChange,
                   ui_->histogram_title_, &QLabel::setText);
  ui_->histogram_widget_->setMouseTracking(true);
}

void OrbitLiveFunctions::Deinitialize() {
  delete all_events_iterator_;
  live_functions_->SetAddIteratorCallback([](uint64_t, const FunctionInfo*) {});
  ui_->data_view_panel_->Deinitialize();
  live_functions_.reset();
}

void OrbitLiveFunctions::SetFilter(const QString& filter) {
  ui_->data_view_panel_->SetFilter(filter);
}

void OrbitLiveFunctions::Refresh() { ui_->data_view_panel_->Refresh(); }

void OrbitLiveFunctions::OnDataChanged() {
  if (live_functions_) {
    live_functions_->OnDataChanged();
  }
}

void OrbitLiveFunctions::AddIterator(size_t id, const FunctionInfo* function) {
  if (!live_functions_) return;

  auto* iterator_ui = new OrbitEventIterator(this);

  iterator_ui->SetNextButtonCallback([this, id]() {
    live_functions_->OnNextButton(id);
    auto it = this->iterator_uis_.find(id);
    it->second->SetCurrentTime(live_functions_->GetStartTime(id));
  });
  iterator_ui->SetPreviousButtonCallback([this, id]() {
    live_functions_->OnPreviousButton(id);
    auto it = this->iterator_uis_.find(id);
    it->second->SetCurrentTime(live_functions_->GetStartTime(id));
  });
  iterator_ui->SetDeleteButtonCallback([this, id]() {
    live_functions_->OnDeleteButton(id);
    auto it = this->iterator_uis_.find(id);
    ui_->iteratorFrame->layout()->removeWidget(it->second);
    it->second->deleteLater();
    iterator_uis_.erase(id);
    if (iterator_uis_.empty()) {
      this->all_events_iterator_->DisableButtons();
    }
  });
  iterator_ui->SetFunctionName(function->pretty_name());

  iterator_ui->SetMinMaxTime(live_functions_->GetCaptureMin(), live_functions_->GetCaptureMax());
  iterator_ui->SetCurrentTime(live_functions_->GetStartTime(id));

  iterator_uis_.insert(std::make_pair(id, iterator_ui));

  all_events_iterator_->EnableButtons();

  dynamic_cast<QBoxLayout*>(ui_->iteratorFrame->layout())
      ->insertWidget(ui_->iteratorFrame->layout()->count() - 1, iterator_ui);

  ui_->tabWidget->setCurrentWidget(ui_->iterators_tab);
}

QLineEdit* OrbitLiveFunctions::GetFilterLineEdit() {
  return ui_->data_view_panel_->GetFilterLineEdit();
}

void OrbitLiveFunctions::Reset() {
  if (!live_functions_) return;

  live_functions_->Reset();

  for (auto& [_, iterator_ui] : iterator_uis_) {
    ui_->iteratorFrame->layout()->removeWidget(iterator_ui);
    delete iterator_ui;
  }
  iterator_uis_.clear();
  all_events_iterator_->DisableButtons();
}

void OrbitLiveFunctions::OnRowSelected(std::optional<int> row) {
  ui_->data_view_panel_->GetTreeView()->SetIsInternalRefresh(true);
  QItemSelectionModel* selection = ui_->data_view_panel_->GetTreeView()->selectionModel();
  QModelIndex index;
  if (row.has_value()) {
    index = ui_->data_view_panel_->GetTreeView()->GetModel()->CreateIndex(row.value(), 0);
  }
  selection->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  ui_->data_view_panel_->GetTreeView()->SetIsInternalRefresh(false);
}

void OrbitLiveFunctions::ShowHistogram(const std::vector<uint64_t>* data, std::string scope_name,
                                       std::optional<orbit_client_data::ScopeId> scope_id) {
  ui_->histogram_widget_->UpdateData(data, std::move(scope_name), scope_id);
}

void OrbitLiveFunctions::SetScopeStatsCollection(
    std::shared_ptr<const orbit_client_data::ScopeStatsCollection> scope_collection) {
  ui_->data_view_panel_->GetTreeView()->selectionModel()->clearSelection();
  live_functions_->SetScopeStatsCollection(std::move(scope_collection));
}