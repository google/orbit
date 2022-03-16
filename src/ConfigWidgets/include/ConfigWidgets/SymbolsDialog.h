// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_SYMBOLS_DIALOG_H_
#define CONFIG_WIDGETS_SYMBOLS_DIALOG_H_

#include <ClientData/ModuleData.h>
#include <absl/types/span.h>

#include <QDialog>
#include <filesystem>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

#include "OrbitBase/Result.h"
#include "SymbolPaths/PersistentStorageManager.h"

namespace Ui {
class SymbolsDialog;  // IWYU pragma: keep
}

namespace orbit_config_widgets {

class SymbolsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SymbolsDialog(orbit_symbol_paths::PersistentStorageManager* persistent_storage_manager,
                         std::optional<const orbit_client_data::ModuleData*> module = std::nullopt,
                         QWidget* parent = nullptr);
  ~SymbolsDialog() override;

  // TryAddSymbolPath will add the path if its not in the list of paths. In case it is, an error
  // message is returned. A path here is either a path to directory, or a path to a file.
  ErrorMessageOr<void> TryAddSymbolPath(const std::filesystem::path& path);
  // TryAddSymbolFile will check whether the following conditions are met and if they are, call
  // TryAddSymbolPath
  // 1. file at file_path is a viable object file.
  // 2. check that the file contains a build-id.
  // 3. if the module_ is set, that the build-id are the same.
  ErrorMessageOr<void> TryAddSymbolFile(const std::filesystem::path& file_path);

 public slots:
  void OnRemoveButtonClicked();
  void OnAddFolderButtonClicked();
  void OnAddFileButtonClicked();
  void OnListItemSelectionChanged();
  void OnMoreInfoButtonClicked();

 private:
  enum class OverrideWarningResult { kOverride, kCancel };

  std::unique_ptr<Ui::SymbolsDialog> ui_;
  std::optional<const orbit_client_data::ModuleData*> module_;
  orbit_symbol_paths::PersistentStorageManager* persistent_storage_manager_ = nullptr;

  void SetSymbolPaths(absl::Span<const std::filesystem::path> paths);
  [[nodiscard]] std::vector<std::filesystem::path> GetSymbolPaths();
  // Returns the caption and file filter for the FileDialog that is opened when the user clicks the
  // "Add File" button.
  [[nodiscard]] std::tuple<QString, QString> GetFilePickerConfig() const;
  [[nodiscard]] OverrideWarningResult DisplayOverrideWarning();
  void SetUpModuleHeadlineLabel();
  void DisableAddFolder();
};

}  // namespace orbit_config_widgets

#endif  // CONFIG_WIDGETS_SYMBOLS_DIALOG_H_