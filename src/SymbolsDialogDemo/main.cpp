// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <filesystem>
#include <vector>

#include "ConfigWidgets/SymbolsDialog.h"
#include "SymbolPaths/PersistentStorageManager.h"

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};
  QApplication::setApplicationName("SymbolsDialogDemo");
  QApplication::setOrganizationName("The Orbit Authors");

  orbit_config_widgets::SymbolsDialog dialog{};
  dialog.SetSymbolPaths(orbit_symbol_paths::LoadPaths());
  const int result_code = dialog.exec();

  if (result_code == QDialog::Accepted) {
    orbit_symbol_paths::SavePaths(dialog.GetSymbolPaths());
  }

  return result_code;
}