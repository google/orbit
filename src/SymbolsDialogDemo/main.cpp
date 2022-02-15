// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <filesystem>
#include <vector>

#include "ConfigWidgets/SymbolsDialog.h"
#include "SymbolPaths/QSettingsBasedStorageManager.h"

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};
  QApplication::setApplicationName("SymbolsDialogDemo");
  QApplication::setOrganizationName("The Orbit Authors");

  orbit_symbol_paths::QSettingsBasedStorageManager symbol_paths_storage_manager;
  orbit_config_widgets::SymbolsDialog dialog{};
  dialog.SetSymbolPaths(symbol_paths_storage_manager.LoadPaths());
  const int result_code = dialog.exec();

  if (result_code == QDialog::Accepted) {
    symbol_paths_storage_manager.SavePaths(dialog.GetSymbolPaths());
  }

  return result_code;
}