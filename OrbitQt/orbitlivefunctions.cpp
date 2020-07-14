// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitlivefunctions.h"

#include "ui_orbitlivefunctions.h"

//-----------------------------------------------------------------------------
OrbitLiveFunctions::OrbitLiveFunctions(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitLiveFunctions) {
  ui->setupUi(this);
  ui->label->hide();

  live_functions_.SetAddIteratorCallback([this](size_t id, Function* function) {
    this->AddIterator(id, function);
  });

  all_events_iterator_ = new OrbitEventIterator();
  all_events_iterator_->SetNextButtonCallback([this]() {
    if (!this->live_functions_.OnAllNextButton()) {
      return;
    }
    for (auto iterator_ui : iterator_uis) {
      iterator_ui.second->IncrementIndex();
    }
  });
  all_events_iterator_->SetPreviousButtonCallback([this]() {
    if (!this->live_functions_.OnAllPreviousButton()) {
      return;
    }
    for (auto iterator_ui : iterator_uis) {
      iterator_ui.second->DecrementIndex();
    }
  });
  all_events_iterator_->SetFunctionName("All functions");
  all_events_iterator_->HideDeleteButton();
  ui->iteratorLayout->addWidget(all_events_iterator_);
}

//-----------------------------------------------------------------------------
OrbitLiveFunctions::~OrbitLiveFunctions() {
  delete ui;
  for (auto it_ui : iterator_uis) {
    delete it_ui.second;
  }
  delete all_events_iterator_;
}

//-----------------------------------------------------------------------------
void OrbitLiveFunctions::Initialize(SelectionType selection_type,
                                    FontType font_type, bool is_main_instance) {
  DataView* data_view = &live_functions_.GetDataView();
  ui->treeView->Initialize(data_view, selection_type, font_type);

  if (is_main_instance) {
    ui->treeView->GetModel()->GetDataView()->SetAsMainInstance();
  }

  std::string label = ui->treeView->GetLabel();
  if (!label.empty()) {
    this->ui->label->setText(QString::fromStdString(label));
    this->ui->label->show();
  }

  data_view->SetUiFilterCallback(
      [this](const std::string& filter) { SetFilter(filter.c_str()); });
}

//-----------------------------------------------------------------------------
void OrbitLiveFunctions::SetFilter(const QString& a_Filter) {
  ui->FilterLineEdit->setText(a_Filter);
  ui->treeView->OnFilter(a_Filter);
}

//-----------------------------------------------------------------------------
void OrbitLiveFunctions::Refresh() { ui->treeView->Refresh(); }

void OrbitLiveFunctions::OnDataChanged() { live_functions_.OnDataChanged(); }

void OrbitLiveFunctions::AddIterator(size_t id, Function* function) {
  OrbitEventIterator* iterator_ui = new OrbitEventIterator();

  iterator_ui->SetNextButtonCallback([this, id]() {
    this->live_functions_.OnNextButton(id);
    auto it = this->iterator_uis.find(id);
    if (it != this->iterator_uis.end()) {
      it->second->IncrementIndex();
    }
  });
  iterator_ui->SetPreviousButtonCallback([this, id]() {
    this->live_functions_.OnPreviousButton(id);
    auto it = this->iterator_uis.find(id);
    if (it != this->iterator_uis.end()) {
      it->second->DecrementIndex();
    }
  });
  iterator_ui->SetDeleteButtonCallback([this, id]() {
    this->live_functions_.OnDeleteButton(id);
    auto it = this->iterator_uis.find(id);
    ui->iteratorLayout->removeWidget(it->second);
    delete it->second;
    iterator_uis.erase(id);
  });
  iterator_ui->SetFunctionName(function->PrettyName());
  iterator_ui->SetMaxCount(function->GetStats().m_Count);
  iterator_ui->SetIndex(0);

  iterator_uis.insert(std::make_pair(id, iterator_ui));

  ui->iteratorLayout->addWidget(iterator_ui);
}
