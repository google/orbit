// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SYMBOLS_Q_SETTINGS_BASED_STORAGE_MANAGER_H_
#define CLIENT_SYMBOLS_Q_SETTINGS_BASED_STORAGE_MANAGER_H_

#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>

#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "ClientSymbols/PersistentStorageManager.h"

namespace orbit_client_symbols {

class QSettingsBasedStorageManager : public PersistentStorageManager {
 public:
  QSettingsBasedStorageManager()
      : QSettingsBasedStorageManager(QCoreApplication::organizationName(),
                                     QCoreApplication::applicationName()) {}

  QSettingsBasedStorageManager(const QString& organization, const QString& application)
      : settings_{organization, application} {}

  void SavePaths(absl::Span<const std::filesystem::path> paths) override;
  [[nodiscard]] std::vector<std::filesystem::path> LoadPaths() override;
  void SaveModuleSymbolFileMappings(const ModuleSymbolFileMappings& mappings) override;
  [[nodiscard]] ModuleSymbolFileMappings LoadModuleSymbolFileMappings() override;
  void SaveDisabledModulePaths(const absl::flat_hash_set<std::string>& paths) override;
  [[nodiscard]] absl::flat_hash_set<std::string> LoadDisabledModulePaths() override;

  void SaveEnableStadiaSymbolStore(bool enable_stadia_symbol_store) override;
  [[nodiscard]] bool LoadEnableStadiaSymbolStore() override;
  void SaveEnableMicrosoftSymbolServer(bool enable_microsoft_symbol_server) override;
  [[nodiscard]] bool LoadEnableMicrosoftSymbolServer() override;

 private:
  QSettings settings_;
};

}  // namespace orbit_client_symbols

#endif  // CLIENT_SYMBOLS_Q_SETTINGS_BASED_STORAGE_MANAGER_H_