// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConfigWidgets/SymbolsDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <algorithm>
#include <memory>

#include "ui_SymbolsDialog.h"

constexpr const char* kFileDialogSavedDirectoryKey = "symbols_file_dialog_saved_directory";

namespace orbit_config_widgets {

SymbolsDialog::~SymbolsDialog() = default;

SymbolsDialog::SymbolsDialog(QWidget* parent)
    : QDialog(parent), ui_(std::make_unique<Ui::SymbolsDialog>()) {
  ui_->setupUi(this);

  QObject::connect(ui_->addButton, &QPushButton::clicked, this, &SymbolsDialog::OnAddButtonClicked);

  QObject::connect(ui_->removeButton, &QPushButton::clicked, this,
                   &SymbolsDialog::OnRemoveButtonClicked);

  QObject::connect(ui_->listWidget, &QListWidget::itemSelectionChanged, this, [this]() {
    ui_->removeButton->setEnabled(!ui_->listWidget->selectedItems().isEmpty());
  });

  // Initially disabled, will be enabled once selection changes.
  ui_->removeButton->setEnabled(false);
}

void SymbolsDialog::SetSymbolPaths(absl::Span<const std::filesystem::path> paths) {
  ui_->listWidget->clear();
  QStringList paths_list;
  paths_list.reserve(static_cast<int>(paths.size()));

  std::transform(
      paths.begin(), paths.end(), std::back_inserter(paths_list),
      [](const std::filesystem::path& path) { return QString::fromStdString(path.string()); });

  ui_->listWidget->addItems(paths_list);
}

void SymbolsDialog::TryAddSymbolPath(const std::filesystem::path& path) {
  QString path_as_qstring = QString::fromStdString(path.string());
  QList<QListWidgetItem*> find_result =
      ui_->listWidget->findItems(path_as_qstring, Qt::MatchFixedString);
  if (find_result.isEmpty()) {
    ui_->listWidget->addItem(path_as_qstring);
    return;
  }

  QMessageBox::warning(this, "Unable to add directory",
                       "The selected directory is already part of the list.");
}

[[nodiscard]] std::vector<std::filesystem::path> SymbolsDialog::GetSymbolPaths() {
  std::vector<std::filesystem::path> result;
  result.reserve(ui_->listWidget->count());

  for (int i = 0; i < ui_->listWidget->count(); ++i) {
    result.emplace_back(ui_->listWidget->item(i)->text().toStdString());
  }

  return result;
}

void SymbolsDialog::OnAddButtonClicked() {
  QSettings settings;
  QString directory = QFileDialog::getExistingDirectory(
      this, "Select Symbol Directory", settings.value(kFileDialogSavedDirectoryKey).toString());
  if (!directory.isEmpty()) {
    settings.setValue(kFileDialogSavedDirectoryKey, directory);
    TryAddSymbolPath(std::filesystem::path{directory.toStdString()});
  }
}

void SymbolsDialog::OnRemoveButtonClicked() {
  for (QListWidgetItem* selected_item : ui_->listWidget->selectedItems()) {
    ui_->listWidget->takeItem(ui_->listWidget->row(selected_item));
  }
}

}  // namespace orbit_config_widgets