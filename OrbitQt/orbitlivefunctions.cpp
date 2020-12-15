// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitlivefunctions.h"

#include "App.h"
#include "ui_orbitlivefunctions.h"

using orbit_client_protos::FunctionInfo;

OrbitLiveFunctions::OrbitLiveFunctions(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitLiveFunctions) {
  ui->setupUi(this);
}

OrbitLiveFunctions::~OrbitLiveFunctions() { delete ui; }

void OrbitLiveFunctions::Initialize(OrbitApp* app, SelectionType selection_type, FontType font_type,
                                    bool is_main_instance) {
  live_functions_.emplace(app);
  DataView* data_view = &live_functions_->GetDataView();
  ui->data_view_panel_->Initialize(data_view, selection_type, font_type, is_main_instance);

  live_functions_->SetAddIteratorCallback(
      [this](size_t id, FunctionInfo* function) { this->AddIterator(id, function); });

  all_events_iterator_ = new OrbitEventIterator(this);
  all_events_iterator_->SetNextButtonCallback([this]() {
    if (!this->live_functions_->OnAllNextButton()) {
      return;
    }
    for (auto& iterator_ui : iterator_uis) {
      uint64_t index = iterator_ui.first;
      iterator_ui.second->SetCurrentTime(this->live_functions_->GetStartTime(index));
    }
  });
  all_events_iterator_->SetPreviousButtonCallback([this]() {
    if (!this->live_functions_->OnAllPreviousButton()) {
      return;
    }
    for (auto& iterator_ui : iterator_uis) {
      uint64_t index = iterator_ui.first;
      iterator_ui.second->SetCurrentTime(this->live_functions_->GetStartTime(index));
    }
  });
  all_events_iterator_->SetFunctionName("All functions");
  all_events_iterator_->HideDeleteButton();
  all_events_iterator_->DisableButtons();
  dynamic_cast<QBoxLayout*>(ui->iteratorFrame->layout())
      ->insertWidget(ui->iteratorFrame->layout()->count() - 1, all_events_iterator_);
}

void OrbitLiveFunctions::SetFilter(const QString& a_Filter) {
  ui->data_view_panel_->SetFilter(a_Filter);
}

void OrbitLiveFunctions::Refresh() { ui->data_view_panel_->Refresh(); }

void OrbitLiveFunctions::OnDataChanged() {
  if (live_functions_) {
    live_functions_->OnDataChanged();
  }
}

void OrbitLiveFunctions::AddIterator(size_t id, FunctionInfo* function) {
  if (!live_functions_) return;

  OrbitEventIterator* iterator_ui = new OrbitEventIterator(this);

  iterator_ui->SetNextButtonCallback([this, id]() {
    live_functions_->OnNextButton(id);
    auto it = this->iterator_uis.find(id);
    it->second->SetCurrentTime(live_functions_->GetStartTime(id));
  });
  iterator_ui->SetPreviousButtonCallback([this, id]() {
    live_functions_->OnPreviousButton(id);
    auto it = this->iterator_uis.find(id);
    it->second->SetCurrentTime(live_functions_->GetStartTime(id));
  });
  iterator_ui->SetDeleteButtonCallback([this, id]() {
    live_functions_->OnDeleteButton(id);
    auto it = this->iterator_uis.find(id);
    ui->iteratorFrame->layout()->removeWidget(it->second);
    it->second->deleteLater();
    iterator_uis.erase(id);
    if (iterator_uis.empty()) {
      this->all_events_iterator_->DisableButtons();
    }
  });
  iterator_ui->SetFunctionName(function->pretty_name());

  iterator_ui->SetMinMaxTime(live_functions_->GetCaptureMin(), live_functions_->GetCaptureMax());
  iterator_ui->SetCurrentTime(live_functions_->GetStartTime(id));

  iterator_uis.insert(std::make_pair(id, iterator_ui));

  all_events_iterator_->EnableButtons();

  dynamic_cast<QBoxLayout*>(ui->iteratorFrame->layout())
      ->insertWidget(ui->iteratorFrame->layout()->count() - 1, iterator_ui);
}

QLineEdit* OrbitLiveFunctions::GetFilterLineEdit() {
  return ui->data_view_panel_->GetFilterLineEdit();
}

void OrbitLiveFunctions::Reset() {
  if (!live_functions_) return;

  live_functions_->Reset();

  for (auto& [_, iterator_ui] : iterator_uis) {
    ui->iteratorFrame->layout()->removeWidget(iterator_ui);
    delete iterator_ui;
  }
  iterator_uis.clear();
  all_events_iterator_->DisableButtons();
}

void OrbitLiveFunctions::OnRowSelected(std::optional<int> row) {
  ui->data_view_panel_->GetTreeView()->SetIsInternalRefresh(true);
  QItemSelectionModel* selection = ui->data_view_panel_->GetTreeView()->selectionModel();
  QModelIndex index;
  if (row.has_value()) {
    index = ui->data_view_panel_->GetTreeView()->GetModel()->CreateIndex(row.value(), 0);
  }
  selection->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  ui->data_view_panel_->GetTreeView()->SetIsInternalRefresh(false);
}
