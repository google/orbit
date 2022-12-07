// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/orbitdataviewpanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <string>
#include <string_view>

#include "ui_orbitdataviewpanel.h"

OrbitDataViewPanel::OrbitDataViewPanel(QWidget* parent)
    : QWidget(parent), ui_(new Ui::OrbitDataViewPanel) {
  ui_->setupUi(this);
  ui_->label->hide();
  connect(ui_->FilterLineEdit, &QLineEdit::textChanged, this,
          &OrbitDataViewPanel::OnFilterLineEditTextChanged);
  connect(ui_->refreshButton, &QPushButton::clicked, this,
          &OrbitDataViewPanel::OnRefreshButtonClicked);
}

OrbitDataViewPanel::~OrbitDataViewPanel() { delete ui_; }

void OrbitDataViewPanel::Initialize(orbit_data_views::DataView* data_view,
                                    SelectionType selection_type, FontType font_type,
                                    bool uniform_row_height,
                                    QFlags<Qt::AlignmentFlag> text_alignment) {
  ui_->treeView->Initialize(data_view, selection_type, font_type, uniform_row_height,
                            text_alignment);

  const std::string label = ui_->treeView->GetLabel();
  if (!label.empty()) {
    this->ui_->label->setText(QString::fromStdString(label));
    this->ui_->label->show();
  }

  if (ui_->treeView->HasRefreshButton()) {
    ui_->refreshButton->show();
    ui_->horizontalLayout->setContentsMargins(0, 0, 6, 0);
  } else {
    ui_->refreshButton->hide();
  }

  data_view->SetUiFilterCallback([this](std::string_view filter) {
    SetFilter(QString::fromUtf8(filter.data(), filter.size()));
  });
}

void OrbitDataViewPanel::Deinitialize() { ui_->treeView->Deinitialize(); }

OrbitTreeView* OrbitDataViewPanel::GetTreeView() { return ui_->treeView; }

QLineEdit* OrbitDataViewPanel::GetFilterLineEdit() { return ui_->FilterLineEdit; }

void OrbitDataViewPanel::Link(OrbitDataViewPanel* panel) {
  ui_->treeView->Link(panel->ui_->treeView);
}

void OrbitDataViewPanel::Refresh() { ui_->treeView->Refresh(); }

void OrbitDataViewPanel::SetDataModel(orbit_data_views::DataView* model) {
  ui_->treeView->SetDataModel(model);
}

void OrbitDataViewPanel::SetFilter(const QString& filter) { ui_->FilterLineEdit->setText(filter); }

void OrbitDataViewPanel::OnFilterLineEditTextChanged(const QString& text) {
  ui_->treeView->OnFilter(text);
}

void OrbitDataViewPanel::OnRefreshButtonClicked() { ui_->treeView->OnRefreshButtonClicked(); }
