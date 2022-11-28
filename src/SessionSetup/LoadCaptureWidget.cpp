// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/LoadCaptureWidget.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QList>
#include <QModelIndex>
#include <QModelIndexList>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QStringList>
#include <QTableView>
#include <QVariant>
#include <Qt>
#include <memory>
#include <tuple>
#include <vector>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "CaptureFileInfo/Manager.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitPaths/Paths.h"
#include "ui_LoadCaptureWidget.h"

namespace orbit_session_setup {

constexpr int kRowHeight = 19;
using orbit_capture_file_info::ItemModel;

// The destructor needs to be defined here because it needs to see the type
// `Ui::LoadCaptureWidget`. The header file only contains a forward declaration.
LoadCaptureWidget::~LoadCaptureWidget() = default;

LoadCaptureWidget::LoadCaptureWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::LoadCaptureWidget>()) {
  orbit_capture_file_info::Manager manager;

  if (manager.GetCaptureFileInfos().empty()) {
    ErrorMessageOr<std::filesystem::path> capture_dir = orbit_paths::CreateOrGetCaptureDir();
    if (capture_dir.has_value()) {
      std::ignore = manager.FillFromDirectory(capture_dir.value());
    }
  }

  item_model_.SetCaptureFileInfos(manager.GetCaptureFileInfos());

  proxy_item_model_.setSourceModel(&item_model_);
  proxy_item_model_.setSortRole(Qt::DisplayRole);
  proxy_item_model_.setFilterCaseSensitivity(Qt::CaseInsensitive);

  ui_->setupUi(this);
  ui_->tableView->setModel(&proxy_item_model_);
  ui_->tableView->setSortingEnabled(true);
  ui_->tableView->sortByColumn(static_cast<int>(ItemModel::Column::kLastUsed),
                               Qt::SortOrder::DescendingOrder);
  ui_->tableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ItemModel::Column::kFilename), QHeaderView::Stretch);
  ui_->tableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ItemModel::Column::kLastUsed), QHeaderView::ResizeToContents);
  ui_->tableView->horizontalHeader()->setSectionResizeMode(
      static_cast<int>(ItemModel::Column::kCreated), QHeaderView::ResizeToContents);
  ui_->tableView->verticalHeader()->setDefaultSectionSize(kRowHeight);

  QObject::connect(ui_->radioButton, &QRadioButton::toggled, this, &LoadCaptureWidget::SetActive);

  QObject::connect(ui_->selectFileButton, &QPushButton::clicked, this,
                   &LoadCaptureWidget::SelectViaFilePicker);

  QObject::connect(ui_->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                   [this](const QItemSelection& selected, const QItemSelection& /*deselected*/) {
                     if (selected.empty()) return;

                     // Since always a whole row is selected, indexes is of size 3, one for each
                     // column. The column does not matter, so column 0 is used.
                     const QModelIndex index = selected.indexes().at(0);

                     ORBIT_CHECK(index.data(Qt::UserRole).canConvert<QString>());

                     emit FileSelected(index.data(Qt::UserRole).value<QString>().toStdString());
                   });

  QObject::connect(ui_->tableView, &QTableView::doubleClicked, this,
                   [this]() { emit SelectionConfirmed(); });

  QObject::connect(ui_->captureFilterLineEdit, &QLineEdit::textChanged, &proxy_item_model_,
                   &QSortFilterProxyModel::setFilterFixedString);
}

void LoadCaptureWidget::SetActive(bool value) {
  ui_->tableContainer->setEnabled(value);
  ui_->selectFileButton->setEnabled(value);
}

void LoadCaptureWidget::SelectViaFilePicker() {
  QString capture_dir;
  ErrorMessageOr<std::filesystem::path> capture_dir_or_error = orbit_paths::CreateOrGetCaptureDir();
  if (capture_dir_or_error.has_value()) {
    capture_dir = QString::fromStdString(capture_dir_or_error.value().string());
  }

  QFileDialog file_picker(this, "Open Capture...", capture_dir, "*.orbit");
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

QRadioButton* LoadCaptureWidget::GetRadioButton() { return ui_->radioButton; }

}  // namespace orbit_session_setup