// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_SYMBOLS_DIALOG_H_
#define CONFIG_WIDGETS_SYMBOLS_DIALOG_H_

#include <absl/types/span.h>

#include <QDialog>
#include <filesystem>
#include <memory>
#include <vector>

namespace Ui {
class SymbolsDialog;  // IWYU pragma: keep
}

namespace orbit_config_widgets {

class SymbolsDialog : public QDialog {
 public:
  explicit SymbolsDialog(QWidget* parent = nullptr);
  ~SymbolsDialog() noexcept override;

  void SetSymbolPaths(absl::Span<const std::filesystem::path> paths);
  // TryAddSymbolPath will only add the path if its not in the list of paths. In case it is, an
  // error message will be displayed to the user instead
  void TryAddSymbolPath(const std::filesystem::path& path);
  [[nodiscard]] std::vector<std::filesystem::path> GetSymbolPaths();

  void OnRemoveButtonClicked();
  void OnAddButtonClicked();

 private:
  std::unique_ptr<Ui::SymbolsDialog> ui_;
};

}  // namespace orbit_config_widgets

#endif  // CONFIG_WIDGETS_SYMBOLS_DIALOG_H_