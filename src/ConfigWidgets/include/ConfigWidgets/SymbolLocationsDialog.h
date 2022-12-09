// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_SYMBOL_LOCATIONS_DIALOG_H_
#define CONFIG_WIDGETS_SYMBOL_LOCATIONS_DIALOG_H_

#include <absl/types/span.h>

#include <QDialog>
#include <QObject>
#include <QString>
#include <QWidget>
#include <filesystem>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientSymbols/PersistentStorageManager.h"
#include "OrbitBase/Result.h"

namespace Ui {
class SymbolLocationsDialog;  // IWYU pragma: keep
}

namespace orbit_config_widgets {

class SymbolLocationsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SymbolLocationsDialog(
      orbit_client_symbols::PersistentStorageManager* persistent_storage_manager,
      bool allow_unsafe_symbols = false,
      std::optional<const orbit_client_data::ModuleData*> module = std::nullopt,
      QWidget* parent = nullptr);
  ~SymbolLocationsDialog() override;

  // TryAddSymbolPath will add the path if its not in the list of paths. In case it is, an error
  // message is returned. A path here is either a path to directory, or a path to a file.
  ErrorMessageOr<void> TryAddSymbolPath(const std::filesystem::path& path);
  // TryAddSymbolFile will add the file_path as a symbol file to the list when possible. The
  // requirements are dependent on the state of this dialog, as follows.
  // 1. If the dialog was opened without a module (not error case), the file will be added (via
  // TryAddSymbolPath), if its a valid symbol file and contains a build id. Otherwise error.
  // 2. If the dialog was opened with a module (the error case) and the Build ID of module and
  // symbols file match, then the file is added (via TryAddSymbolPath). Otherwise:
  // 3. If the Build IDs dont match, and only safe symbols are allowed (allow_unsafe_symbols_ ==
  // false), then this will return an error. If unsafe symbols are allowed, this function will
  // display a message box where the user can choose an "symbol file override". In this case the
  // "module symbol file mapping" is saved and added to the list.
  ErrorMessageOr<void> TryAddSymbolFile(const std::filesystem::path& file_path);

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 public slots:  // NOLINT(readability-redundant-access-specifiers)
  void OnRemoveButtonClicked();
  void OnAddFolderButtonClicked();
  void OnAddFileButtonClicked();
  void OnListItemSelectionChanged();
  void OnMoreInfoButtonClicked();

 private:
  enum class OverrideWarningResult { kOverride, kCancel };

  std::unique_ptr<Ui::SymbolLocationsDialog> ui_;
  bool allow_unsafe_symbols_;
  std::optional<const orbit_client_data::ModuleData*> module_;
  orbit_client_symbols::PersistentStorageManager* persistent_storage_manager_ = nullptr;
  orbit_client_symbols::ModuleSymbolFileMappings module_symbol_file_mappings_;

  void AddSymbolPathsToListWidget(absl::Span<const std::filesystem::path> paths);
  [[nodiscard]] std::vector<std::filesystem::path> GetSymbolPathsFromListWidget();
  void AddModuleSymbolFileMappingsToList();
  ErrorMessageOr<void> AddMapping(const orbit_client_data::ModuleData& module,
                                  const std::filesystem::path& symbol_file_path);

  // Returns the caption and file filter for the FileDialog that is opened when the user clicks the
  // "Add File" button.
  [[nodiscard]] std::tuple<QString, QString> GetFilePickerConfig() const;
  [[nodiscard]] OverrideWarningResult DisplayOverrideWarning();
  void SetUpModuleHeadlineLabel();
  void DisableAddFolder();
  void SetUpInfoLabel();
};

}  // namespace orbit_config_widgets

#endif  // CONFIG_WIDGETS_SYMBOL_LOCATIONS_DIALOG_H_