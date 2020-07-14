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

  live_functions_.SetAddIteratorCallback(
      [this](Function* function) { this->AddIterator(function); });

  all_events_iterator_ = new OrbitEventIterator();
  all_events_iterator_->SetNextButtonCallback(
      [this]() { this->live_functions_.OnAllNextButton(); });
  all_events_iterator_->SetPreviousButtonCallback(
      [this]() { this->live_functions_.OnAllPreviousButton(); });
  all_events_iterator_->SetFunctionName("All functions");
  ui->iteratorLayout->addWidget(all_events_iterator_);
}

//-----------------------------------------------------------------------------
OrbitLiveFunctions::~OrbitLiveFunctions() {
  delete ui;
  for (auto it_ui : iterator_uis) {
    delete it_ui;
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

void OrbitLiveFunctions::AddIterator(Function* function) {
  OrbitEventIterator* iterator_ui = new OrbitEventIterator();
  size_t new_index = iterator_uis.size();
  iterator_ui->SetNextButtonCallback([this, new_index]() {
    this->live_functions_.OnNextButton(new_index);
    this->iterator_uis[new_index]->IncrementIndex();
  });
  iterator_ui->SetPreviousButtonCallback([this, new_index]() {
    this->live_functions_.OnPreviousButton(new_index);
    this->iterator_uis[new_index]->DecrementIndex();
  });
  iterator_ui->SetFunctionName(function->PrettyName());
  iterator_ui->SetMaxCount(function->GetStats().m_Count);
  iterator_ui->SetIndex(0);
  iterator_uis.push_back(iterator_ui);
  ui->iteratorLayout->addWidget(iterator_ui);
}
