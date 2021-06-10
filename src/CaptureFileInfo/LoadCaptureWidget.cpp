// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFileInfo/LoadCaptureWidget.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <memory>

#include "CaptureFileInfo/Manager.h"
#include "OrbitBase/Logging.h"
#include "Path.h"
#include "ui_LoadCaptureWidget.h"

constexpr int kRowHeight = 19;

namespace orbit_capture_file_info {

// The destructor needs to be defined here because it needs to see the type
// `Ui::LoadCaptureWidget`. The header file only contains a forward declaration.
LoadCaptureWidget::~LoadCaptureWidget() noexcept = default;

LoadCaptureWidget::LoadCaptureWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::LoadCaptureWidget>()) {
  Manager manager;

  if (manager.GetCaptureFileInfos().empty()) {
    (void)manager.FillFromDirectory(orbit_core::CreateOrGetCaptureDir());
  }

  item_model_.SetCaptureFileInfos(manager.GetCaptureFileInfos());

  proxy_item_model_.setSourceModel(&item_model_);
  proxy_item_model_.setSortRole(Qt::DisplayRole);

  ui_->setupUi(this);
  ui_->tableView->setModel(&proxy_item_model_);
  ui_->tableView->setSortingEnabled(true);
  ui_->tableView->sortByColumn(static_cast<int>(ItemModel::Column::kLastUsed),
                               Qt::SortOrder::DescendingOrder);
  ui_->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  ui_->tableView->verticalHeader()->setDefaultSectionSize(kRowHeight);

  // The following is to make the radiobutton behave as if it was part of an exclusive button group
  // in the parent widget (ProfilingTargetDialog). If a user clicks on the radiobutton and it was
  // not checked before, it is checked afterwards and this widget sends the activation signal.
  // ProfilingTargetDialog reacts to the signal and deactivates the other widgets belonging to that
  // button group. If a user clicks on a radio button that is already checked, nothing happens, the
  // button does not get unchecked.
  QObject::connect(ui_->radioButton, &QRadioButton::clicked, this, [this](bool checked) {
    if (checked) {
      emit Activated();
    } else {
      ui_->radioButton->setChecked(true);
    }
  });

  QObject::connect(ui_->selectFileButton, &QPushButton::clicked, this,
                   &LoadCaptureWidget::SelectViaFilePicker);

  QObject::connect(ui_->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                   [this](const QItemSelection& selected, const QItemSelection& /*deselected*/) {
                     if (selected.empty()) return;

                     // Since always a whole row is selected, indexes is of size 3, one for each
                     // column. The column does not matter, so column 0 is used.
                     const QModelIndex index = selected.indexes().at(0);

                     CHECK(index.data(Qt::UserRole).canConvert<QString>());

                     emit FileSelected(index.data(Qt::UserRole).value<QString>().toStdString());
                   });

  QObject::connect(ui_->tableView, &QTableView::doubleClicked, this,
                   [this]() { emit SelectionConfirmed(); });
}

bool LoadCaptureWidget::IsActive() const { return ui_->contentFrame->isEnabled(); }

void LoadCaptureWidget::SetActive(bool value) {
  ui_->contentFrame->setEnabled(value);
  ui_->radioButton->setChecked(value);
}

void LoadCaptureWidget::DetachRadioButton() {
  ui_->titleBarLayout->removeWidget(ui_->radioButton);
  ui_->radioButton->setParent(ui_->mainFrame);
  int left = 0;
  int top = 0;
  ui_->mainFrame->layout()->getContentsMargins(&left, &top, nullptr, nullptr);
  int frame_border_width = ui_->mainFrame->lineWidth();
  ui_->radioButton->move(left + frame_border_width, top + frame_border_width);
  ui_->radioButton->show();
}

void LoadCaptureWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  // It is important that the call to DetachRadioButton is done here and not during construction.
  // For high dpi display settings in Windows (scaling) the the actual width and height of the radio
  // button is not known during construction. Hence the call is done when the widget is shown, not
  // when its constructed.
  DetachRadioButton();
}

void LoadCaptureWidget::SelectViaFilePicker() {
  QFileDialog file_picker(this, "Open Capture...",
                          QString::fromStdString(orbit_core::CreateOrGetCaptureDir().string()),
                          "*.orbit");
  file_picker.setFileMode(QFileDialog::ExistingFile);
  file_picker.setLabelText(QFileDialog::Accept, "Start Session");

  int rc = file_picker.exec();
  if (rc == 0) return;

  // Since QFileDialog::ExistingFile (instead of ExistingFile*s*) is used, it will always be exactly
  // one selected file.
  const QString file_path = file_picker.selectedFiles()[0];

  ui_->tableView->clearSelection();

  emit FileSelected(file_path.toStdString());
  emit SelectionConfirmed();
}

}  // namespace orbit_capture_file_info