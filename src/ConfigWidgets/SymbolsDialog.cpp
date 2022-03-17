// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConfigWidgets/SymbolsDialog.h"

#include <absl/strings/str_format.h>

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <tuple>

#include "GrpcProtos/module.pb.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "ui_SymbolsDialog.h"

constexpr const char* kFileDialogSavedDirectoryKey = "symbols_file_dialog_saved_directory";
constexpr const char* kModuleHeadlineLabel = "Add Symbols for <font color=\"#E64646\">%1</font>";
constexpr const char* kOverrideWarningText =
    "The Build ID in the file you selected does not match. This may lead to unexpected behavior in "
    "Orbit.<br />Override to use this file.";

using orbit_client_data::ModuleData;
using orbit_grpc_protos::ModuleInfo;

namespace orbit_config_widgets {

SymbolsDialog::~SymbolsDialog() { persistent_storage_manager_->SavePaths(GetSymbolPaths()); }

SymbolsDialog::SymbolsDialog(
    orbit_symbol_paths::PersistentStorageManager* persistent_storage_manager,
    std::optional<const ModuleData*> module, QWidget* parent)
    : QDialog(parent),
      ui_(std::make_unique<Ui::SymbolsDialog>()),
      module_(module),
      persistent_storage_manager_(persistent_storage_manager) {
  ORBIT_CHECK(persistent_storage_manager_ != nullptr);
  ui_->setupUi(this);

  SetSymbolPaths(persistent_storage_manager_->LoadPaths());

  if (!module_.has_value()) return;

  SetUpModuleHeadlineLabel();

  if (!module_.value()->build_id().empty()) return;

  // To find a symbols in a symbol folder, the build id of module and potential symbols file are
  // matched. Therefore, if the build id of the module is empty, Orbit will never be able to match
  // a symbols file. So adding a symbol folder is disabled here when the module does not have a
  // build ID.
  DisableAddFolder();
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

ErrorMessageOr<void> SymbolsDialog::TryAddSymbolPath(const std::filesystem::path& path) {
  QString path_as_qstring = QString::fromStdString(path.string());
  QList<QListWidgetItem*> find_result =
      ui_->listWidget->findItems(path_as_qstring, Qt::MatchFixedString);
  if (!find_result.isEmpty()) {
    return ErrorMessage("Unable to add selected path, it is already part of the list.");
  }

  ui_->listWidget->addItem(path_as_qstring);
  return outcome::success();
}

[[nodiscard]] std::vector<std::filesystem::path> SymbolsDialog::GetSymbolPaths() {
  std::vector<std::filesystem::path> result;
  result.reserve(ui_->listWidget->count());

  for (int i = 0; i < ui_->listWidget->count(); ++i) {
    result.emplace_back(ui_->listWidget->item(i)->text().toStdString());
  }

  return result;
}

void SymbolsDialog::OnAddFolderButtonClicked() {
  QSettings settings;
  QString directory = QFileDialog::getExistingDirectory(
      this, "Select Symbol Folder", settings.value(kFileDialogSavedDirectoryKey).toString());
  if (directory.isEmpty()) return;

  settings.setValue(kFileDialogSavedDirectoryKey, directory);
  ErrorMessageOr<void> result = TryAddSymbolPath(std::filesystem::path{directory.toStdString()});
  if (!result.has_error()) return;

  QMessageBox::warning(this, "Unable to add folder",
                       QString::fromStdString(result.error().message()));
}

void SymbolsDialog::OnRemoveButtonClicked() {
  for (QListWidgetItem* selected_item : ui_->listWidget->selectedItems()) {
    ui_->listWidget->takeItem(ui_->listWidget->row(selected_item));
  }
}

std::tuple<QString, QString> SymbolsDialog::GetFilePickerConfig() const {
  QString file_filter{"Symbol Files (*.debug *.so *.pdb *.dll);;All files (*)"};

  if (!module_.has_value()) {
    return std::make_tuple("Select symbol file", file_filter);
  }

  const ModuleData& module{*module_.value()};

  QString caption =
      QString("Select symbol file for module %1").arg(QString::fromStdString(module.name()));

  switch (module.object_file_type()) {
    case ModuleInfo::kElfFile:
      file_filter = "Symbol Files (*.debug *.so);;All files (*)";
      break;
    case ModuleInfo::kCoffFile:
      file_filter = "Symbol Files (*.pdb, *.dll);;All files (*)";
      break;
    default:
      ORBIT_ERROR("Cant determine file picker filter: unknown module type");
      break;
  }

  return std::make_tuple(caption, file_filter);
}

void SymbolsDialog::OnAddFileButtonClicked() {
  QSettings settings;

  auto [caption, file_filter] = GetFilePickerConfig();

  QString file = QFileDialog::getOpenFileName(
      this, caption, settings.value(kFileDialogSavedDirectoryKey).toString(), file_filter);
  if (file.isEmpty()) return;

  std::filesystem::path path{file.toStdString()};

  settings.setValue(kFileDialogSavedDirectoryKey,
                    QString::fromStdString(path.parent_path().string()));
  ErrorMessageOr<void> add_result = TryAddSymbolFile(path);

  if (!add_result.has_error()) return;

  QMessageBox::warning(this, "Unable to add file",
                       QString::fromStdString(add_result.error().message()));
}

ErrorMessageOr<void> SymbolsDialog::TryAddSymbolFile(const std::filesystem::path& file_path) {
  // ObjectFileInfo is only required when loading symbols from the file. Since here it is only
  // created to check whether the file is valid (and not to actually load symbols), a default
  // constructed ObjectFileInfo can be used.
  ErrorMessageOr<std::unique_ptr<orbit_object_utils::SymbolsFile>> symbols_file_or_error =
      orbit_object_utils::CreateSymbolsFile(file_path, orbit_object_utils::ObjectFileInfo());

  if (symbols_file_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("The selected file is not a viable symbol file, error: %s",
                                        symbols_file_or_error.error().message()));
  }

  const orbit_object_utils::SymbolsFile& symbols_file{*symbols_file_or_error.value()};

  const std::string build_id = symbols_file.GetBuildId();
  if (build_id.empty()) {
    return ErrorMessage("The selected file does not contain a build id");
  }

  // In case this dialog was not opened with a module, there is no reason to compare build-ids.
  // Hence the path of the symbols file is added to this dialog.
  if (!module_.has_value()) return TryAddSymbolPath(file_path);

  const ModuleData& module{*module_.value()};

  if (module.build_id() == build_id) return TryAddSymbolPath(file_path);

  std::string error = absl::StrFormat(
      "The build ids of module and symbols file do not match. Module (%s) build id: \"%s\". Symbol "
      "file (%s) build id: \"%s\".",
      module.file_path(), module.build_id(), file_path.string(), build_id);
  return ErrorMessage(error);
}

void SymbolsDialog::OnListItemSelectionChanged() {
  ui_->removeButton->setEnabled(!ui_->listWidget->selectedItems().isEmpty());
}

void SymbolsDialog::OnMoreInfoButtonClicked() {
  QString url_as_string{
      "https://developers.google.com/stadia/docs/develop/optimize/"
      "profile-cpu-with-orbit#load_symbols"};
  if (!QDesktopServices::openUrl(QUrl(url_as_string, QUrl::StrictMode))) {
    QMessageBox::critical(this, "Error opening URL",
                          QString("Could not open %1").arg(url_as_string));
  }
}

[[nodiscard]] SymbolsDialog::OverrideWarningResult SymbolsDialog::DisplayOverrideWarning() {
  QMessageBox message_box{QMessageBox::Warning, "Override Symbol location?", kOverrideWarningText,
                          QMessageBox::StandardButton::Cancel, this};
  message_box.addButton("Override", QMessageBox::AcceptRole);
  int result_code = message_box.exec();
  if (result_code == QMessageBox::Accepted) {
    return OverrideWarningResult::kOverride;
  }
  return OverrideWarningResult::kCancel;
}

void SymbolsDialog::SetUpModuleHeadlineLabel() {
  ORBIT_CHECK(module_.has_value());
  ui_->moduleHeadlineLabel->setVisible(true);
  ui_->moduleHeadlineLabel->setText(
      QString(kModuleHeadlineLabel).arg(QString::fromStdString(module_.value()->name())));
}

void SymbolsDialog::DisableAddFolder() {
  ORBIT_CHECK(module_.has_value());

  ui_->addFolderButton->setDisabled(true);
  ui_->addFolderButton->setToolTip(
      QString("Module %1 does not have a build ID. For modules without build ID, Orbit cannot find "
              "symbols in folders.")
          .arg(QString::fromStdString(module_.value()->name())));
}

}  // namespace orbit_config_widgets