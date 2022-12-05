// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConfigWidgets/SymbolLocationsDialog.h"

#include <absl/container/flat_hash_map.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>

#include <QAbstractButton>
#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <Qt>
#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include "ClientFlags/ClientFlags.h"
#include "GrpcProtos/module.pb.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "ui_SymbolLocationsDialog.h"

constexpr const char* kFileDialogSavedDirectoryKey = "symbols_file_dialog_saved_directory";
constexpr const char* kModuleHeadlineLabel =
    "Orbit was not able to load symbols for <font color=\"#E64646\">%1</font>";
constexpr const char* kOverrideWarningText =
    "The Build ID in the file you selected does not match. This may lead to unexpected behavior in "
    "Orbit.<br />Override to use this file.";
constexpr const char* kNewInfoLabelTemplate =
    "<p><b>Add Folder</b> to add a symbol location. The symbol files' filenames and build IDs must "
    "match the module's name and build ID. Supported file extensions are “.so”, “.debug”, "
    "“.so.debug”, “.dll” and “.pdb”.</p><p><b>Add File</b> to load from a symbol file with a "
    "different filename%1</p>";
constexpr const char* kInfoLabelArgumentNoBuildIdOverride = " or extension.";
constexpr const char* kInfoLabelArgumentWithBuildIdOverride = ", extension or build ID.";
constexpr QListWidgetItem::ItemType kOverrideMappingItemType = QListWidgetItem::ItemType::UserType;

using orbit_client_data::ModuleData;
using orbit_grpc_protos::ModuleInfo;
using orbit_object_utils::SymbolsFile;

namespace {

// OverrideMappingItem is a class to represent an override (module to symbol file mapping) entry in
// the Symbol Locations list. It subclasses QListWidgetItem so it can be added to the QListWidget
// and distinguished from the regular path entries, that are "simple" QListWidgetItems. An
// OverrideMappingItem carries an alert icon which is displayed at the beginning of the line. It
// also has an explanatory tooltip and saves the module file path, so the
// SymbolLocationsDialog::OnRemoveButtonClicked can delete the corresponding entry from the
// module_symbol_file_mappings_ map.
class OverrideMappingItem : public QListWidgetItem {
 public:
  explicit OverrideMappingItem(std::string_view module_file_path,
                               const std::filesystem::path& symbol_file_path,
                               QListWidget* parent = nullptr)
      : QListWidgetItem(
            QIcon(":/actions/alert"),
            QString("%1 -> %2")
                .arg(QString::fromUtf8(module_file_path.data(), module_file_path.size()))
                .arg(QString::fromStdString(symbol_file_path.string())),
            parent, kOverrideMappingItemType),
        module_file_path_(module_file_path) {
    setToolTip(
        QString(
            R"(This is a symbol file override. Orbit will always use the symbol file "%1" for the module "%2".)")
            .arg(QString::fromUtf8(module_file_path.data(), module_file_path.size()))
            .arg(QString::fromStdString(symbol_file_path.string())));
  }
  std::string module_file_path_;
};

ErrorMessageOr<std::unique_ptr<SymbolsFile>> CreateValidSymbolsFile(
    const std::filesystem::path& file_path) {
  // ObjectFileInfo is only required when loading symbols from the file. Since here it is only
  // created to check whether the file is valid (and not to actually load symbols), a default
  // constructed ObjectFileInfo can be used.
  ErrorMessageOr<std::unique_ptr<SymbolsFile>> symbols_file_or_error =
      orbit_object_utils::CreateSymbolsFile(file_path, orbit_object_utils::ObjectFileInfo());

  if (symbols_file_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("The selected file is not a viable symbol file, error: %s",
                                        symbols_file_or_error.error().message()));
  }
  return symbols_file_or_error;
}

ErrorMessageOr<void> CheckValidSymbolsFileWithBuildId(const std::filesystem::path& file_path) {
  OUTCOME_TRY(std::unique_ptr<SymbolsFile> symbols_file, CreateValidSymbolsFile(file_path));

  if (symbols_file->GetBuildId().empty()) {
    return ErrorMessage("The selected file does not contain a build id");
  }
  return outcome::success();
}

}  // namespace

namespace orbit_config_widgets {

SymbolLocationsDialog::~SymbolLocationsDialog() {
  persistent_storage_manager_->SavePaths(GetSymbolPathsFromListWidget());
  persistent_storage_manager_->SaveModuleSymbolFileMappings(module_symbol_file_mappings_);

  if (absl::GetFlag(FLAGS_symbol_store_support)) {
    persistent_storage_manager_->SaveEnableStadiaSymbolStore(
        ui_->enableStadiaSymbolStoreCheckBox->isChecked());
    persistent_storage_manager_->SaveEnableMicrosoftSymbolServer(
        ui_->enableMicrosoftSymbolServerCheckBox->isChecked());
  }
}

SymbolLocationsDialog::SymbolLocationsDialog(
    orbit_client_symbols::PersistentStorageManager* persistent_storage_manager,
    bool allow_unsafe_symbols, std::optional<const ModuleData*> module, QWidget* parent)
    : QDialog(parent),
      ui_(std::make_unique<Ui::SymbolLocationsDialog>()),
      allow_unsafe_symbols_(allow_unsafe_symbols),
      module_(module),
      persistent_storage_manager_(persistent_storage_manager),
      module_symbol_file_mappings_(persistent_storage_manager_->LoadModuleSymbolFileMappings()) {
  ORBIT_CHECK(persistent_storage_manager_ != nullptr);

  // When the symbols dialog is started with a module (from the error) *and* only save symbols are
  // allowed, then the module is required to have a build ID. Without a build ID Orbit will not be
  // able to match any symbol file. This is enforced, because in SymbolErrorDialog the "Add Symbol
  // Location" button is disabled when the module does not have a build id (and only safe symbols
  // are allowed).
  if (module_.has_value() && !allow_unsafe_symbols) {
    ORBIT_CHECK(!module_.value()->build_id().empty());
  }

  ui_->setupUi(this);
  SetUpInfoLabel();
  if (!absl::GetFlag(FLAGS_symbol_store_support)) {
    ui_->symbolStoreGroupBox->hide();
  } else {
    ui_->enableStadiaSymbolStoreCheckBox->setChecked(
        persistent_storage_manager_->LoadEnableStadiaSymbolStore());
    ui_->enableMicrosoftSymbolServerCheckBox->setChecked(
        persistent_storage_manager_->LoadEnableMicrosoftSymbolServer());
  }

  if (allow_unsafe_symbols_) AddModuleSymbolFileMappingsToList();
  AddSymbolPathsToListWidget(persistent_storage_manager_->LoadPaths());

  if (!module_.has_value()) {
    return;
  }

  SetUpModuleHeadlineLabel();

  if (!module_.value()->build_id().empty()) return;

  // To find a symbols in a symbol folder, the build id of module and potential symbols file are
  // matched. Therefore, if the build id of the module is empty, Orbit will never be able to match
  // a symbols file. So adding a symbol folder is disabled here when the module does not have a
  // build ID.
  DisableAddFolder();
}

void SymbolLocationsDialog::AddSymbolPathsToListWidget(
    absl::Span<const std::filesystem::path> paths) {
  QStringList paths_list;
  paths_list.reserve(static_cast<int>(paths.size()));

  std::transform(
      paths.begin(), paths.end(), std::back_inserter(paths_list),
      [](const std::filesystem::path& path) { return QString::fromStdString(path.string()); });

  ui_->listWidget->addItems(paths_list);
}

ErrorMessageOr<void> SymbolLocationsDialog::TryAddSymbolPath(const std::filesystem::path& path) {
  QString path_as_qstring = QString::fromStdString(path.string());
  QList<QListWidgetItem*> find_result =
      ui_->listWidget->findItems(path_as_qstring, Qt::MatchFixedString);
  if (!find_result.isEmpty()) {
    return ErrorMessage("Unable to add selected path, it is already part of the list.");
  }

  ui_->listWidget->addItem(path_as_qstring);
  return outcome::success();
}

[[nodiscard]] std::vector<std::filesystem::path>
SymbolLocationsDialog::GetSymbolPathsFromListWidget() {
  std::vector<std::filesystem::path> result;

  for (int i = 0; i < ui_->listWidget->count(); ++i) {
    QListWidgetItem* item = ui_->listWidget->item(i);
    ORBIT_CHECK(item != nullptr);
    if (item->type() == kOverrideMappingItemType) continue;

    result.emplace_back(item->text().toStdString());
  }

  return result;
}

void SymbolLocationsDialog::OnAddFolderButtonClicked() {
  QSettings settings;
  QString directory = QFileDialog::getExistingDirectory(
      this, "Select Symbol Folder", settings.value(kFileDialogSavedDirectoryKey).toString());
  if (directory.isEmpty()) return;

  settings.setValue(kFileDialogSavedDirectoryKey, directory);

  ErrorMessageOr<void> add_result = ErrorMessage{""};

  add_result = TryAddSymbolPath(std::filesystem::path{directory.toStdString()});
  if (!add_result.has_error()) return;

  QMessageBox::warning(this, "Unable to add folder",
                       QString::fromStdString(add_result.error().message()));
}

void SymbolLocationsDialog::OnRemoveButtonClicked() {
  for (QListWidgetItem* selected_item : ui_->listWidget->selectedItems()) {
    if (selected_item->type() == kOverrideMappingItemType) {
      auto* mapping_item = dynamic_cast<OverrideMappingItem*>(selected_item);
      ORBIT_CHECK(mapping_item != nullptr);
      ORBIT_CHECK(module_symbol_file_mappings_.contains(mapping_item->module_file_path_));
      module_symbol_file_mappings_.erase(mapping_item->module_file_path_);
    }

    // This object is managed by Qt. A "raw" delete is unavoidable.
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete selected_item;
  }
}

std::tuple<QString, QString> SymbolLocationsDialog::GetFilePickerConfig() const {
  QString file_filter{"Symbol Files (*.debug *.so *.pdb *.dll);;All files (*)"};

  if (!module_.has_value()) {
    return std::make_tuple("Select symbol file", file_filter);
  }

  const ModuleData& module{*module_.value()};

  QString caption = QString("Select symbol file for module %1")
                        .arg(QString::fromStdString(
                            std::filesystem::path(module.file_path()).filename().string()));

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

void SymbolLocationsDialog::OnAddFileButtonClicked() {
  QSettings settings;

  auto [caption, file_filter] = GetFilePickerConfig();

  QString file = QFileDialog::getOpenFileName(
      this, caption, settings.value(kFileDialogSavedDirectoryKey).toString(), file_filter);
  if (file.isEmpty()) return;

  std::filesystem::path path{file.toStdString()};

  settings.setValue(kFileDialogSavedDirectoryKey,
                    QString::fromStdString(path.parent_path().string()));
  ErrorMessageOr<void> add_path_result = TryAddSymbolFile(path);

  if (!add_path_result.has_error()) return;

  QMessageBox::warning(this, "Unable to add file",
                       QString::fromStdString(add_path_result.error().message()));
}

ErrorMessageOr<void> SymbolLocationsDialog::TryAddSymbolFile(
    const std::filesystem::path& file_path) {
  // If the dialog was opened without a module, every valid symbols file with build id can be
  // added
  if (!module_.has_value()) {
    OUTCOME_TRY(CheckValidSymbolsFileWithBuildId(file_path));

    return TryAddSymbolPath(file_path);
  }

  const ModuleData& module{*module_.value()};

  OUTCOME_TRY(const std::unique_ptr<SymbolsFile> symbols_file, CreateValidSymbolsFile(file_path));

  // If the build ids match, the file can be used
  if (!module.build_id().empty() && module.build_id() == symbols_file->GetBuildId()) {
    return TryAddSymbolPath(file_path);
  }

  // If only safe symbols are allowed, then a mismatching error is returned here
  if (!allow_unsafe_symbols_) {
    const std::string error = absl::StrFormat(
        "The build ids of module and symbols file do not match. Module (%s) build id: \"%s\". "
        "Symbol file (%s) build id: \"%s\".",
        module.file_path(), module.build_id(), file_path.string(), symbols_file->GetBuildId());
    return ErrorMessage(error);
  }

  const OverrideWarningResult override_result = DisplayOverrideWarning();

  switch (override_result) {
    case OverrideWarningResult::kOverride:
      return AddMapping(module, file_path);
    case OverrideWarningResult::kCancel: {
      // success here means "no error", aka the symbol file adding ended without an error (was
      // cancelled)
      return outcome::success();
    }
  }
  ORBIT_UNREACHABLE();
}

void SymbolLocationsDialog::OnListItemSelectionChanged() {
  ui_->removeButton->setEnabled(!ui_->listWidget->selectedItems().isEmpty());
}

void SymbolLocationsDialog::OnMoreInfoButtonClicked() {
  QString url_as_string{
      "https://github.com/google/orbit/blob/main/documentation/DOCUMENTATION.md#load-symbols"};
  if (!QDesktopServices::openUrl(QUrl(url_as_string, QUrl::StrictMode))) {
    QMessageBox::critical(this, "Error opening URL",
                          QString("Could not open %1").arg(url_as_string));
  }
}

[[nodiscard]] SymbolLocationsDialog::OverrideWarningResult
SymbolLocationsDialog::DisplayOverrideWarning() {
  QMessageBox message_box{QMessageBox::Warning, "Override Symbol location?", kOverrideWarningText,
                          QMessageBox::StandardButton::Cancel, this};
  QAbstractButton* override_button = message_box.addButton("Override", QMessageBox::AcceptRole);

  // From https://doc.qt.io/qt-5/qmessagebox.html#exec
  // > When using QMessageBox with custom buttons, this function [exec] returns an opaque value;
  // use clickedButton() to determine which button was clicked.
  (void)message_box.exec();
  if (message_box.clickedButton() == override_button) {
    return OverrideWarningResult::kOverride;
  }
  return OverrideWarningResult::kCancel;
}

void SymbolLocationsDialog::AddModuleSymbolFileMappingsToList() {
  for (const auto& [module_path, symbol_file_path] : module_symbol_file_mappings_) {
    // The "new" here is okay, because listWidget will own the MappingItem and the Qt lifecycle
    // management will take care of its deletion
    ui_->listWidget->addItem(new OverrideMappingItem(module_path, symbol_file_path));
  }
}

ErrorMessageOr<void> SymbolLocationsDialog::AddMapping(
    const ModuleData& module, const std::filesystem::path& symbol_file_path) {
  if (module_symbol_file_mappings_.contains(module.file_path())) {
    return ErrorMessage(
        absl::StrFormat("Module \"%s\" is already mapped to a symbol file (\"%s\"). Please remove "
                        "the existing mapping before adding a new one.",
                        module.file_path(), symbol_file_path.string()));
  }

  module_symbol_file_mappings_[module.file_path()] = symbol_file_path;
  ui_->listWidget->addItem(new OverrideMappingItem(module.file_path(), symbol_file_path));
  return outcome::success();
}

void SymbolLocationsDialog::SetUpModuleHeadlineLabel() {
  ORBIT_CHECK(module_.has_value());
  ui_->moduleHeadlineLabel->setVisible(true);
  ui_->moduleHeadlineLabel->setText(
      QString(kModuleHeadlineLabel)
          .arg(QString::fromStdString(
              std::filesystem::path(module_.value()->file_path()).filename().string())));
  ui_->line->setVisible(true);
}

void SymbolLocationsDialog::DisableAddFolder() {
  ORBIT_CHECK(module_.has_value());

  ui_->addFolderButton->setDisabled(true);
  ui_->addFolderButton->setToolTip(
      QString("Module %1 does not have a build ID. For modules without build ID, Orbit cannot find "
              "symbols in folders.")
          .arg(QString::fromStdString(
              std::filesystem::path(module_.value()->file_path()).filename().string())));
}

void SymbolLocationsDialog::SetUpInfoLabel() {
  QString label_text = kNewInfoLabelTemplate;
  if (allow_unsafe_symbols_) {
    label_text = label_text.arg(kInfoLabelArgumentWithBuildIdOverride);
  } else {
    label_text = label_text.arg(kInfoLabelArgumentNoBuildIdOverride);
  }
  ui_->infoLabel->setText(label_text);
}

}  // namespace orbit_config_widgets