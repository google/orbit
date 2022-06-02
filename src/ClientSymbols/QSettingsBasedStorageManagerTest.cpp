// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>
#include <filesystem>
#include <memory>

#include "ClientSymbols/QSettingsBasedStorageManager.h"

const std::filesystem::path path0{"/path/to/symbols/path"};
const std::filesystem::path path1{"/home/src/project/build/"};
const std::filesystem::path path2{R"(c:\project\build\)"};

constexpr const char* kOrgName = "The Orbit Authors";

namespace orbit_client_symbols {

TEST(QSettingsBasedStorageManager, LoadAndSavePaths) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("QSettingsBasedStorageManager.LoadAndSavePaths");

  {  // clear before test;
    QSettings settings;
    settings.clear();
  }

  QSettingsBasedStorageManager manager;

  EXPECT_EQ(manager.LoadPaths(), std::vector<std::filesystem::path>{});

  std::vector<std::filesystem::path> paths{
      path0,
      path1,
      path2,
  };

  manager.SavePaths(paths);
  EXPECT_EQ(manager.LoadPaths(), paths);
}

TEST(QSettingsBasedStorageManager, LoadAndSaveModuleSymbolFileMappings) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName(
      "QSettingsBasedStorageManager.LoadAndSaveModuleSymbolFileMappings");

  {  // clear before test;
    QSettings settings;
    settings.clear();
  }

  absl::flat_hash_map<std::string, std::filesystem::path> mappings;
  mappings["/path/to/module1"] = path0;
  mappings["/path/to/module2"] = path1;
  mappings["/other/module/path"] = path2;

  {
    QSettingsBasedStorageManager manager;
    EXPECT_TRUE(manager.LoadModuleSymbolFileMappings().empty());
    manager.SaveModuleSymbolFileMappings(mappings);
  }

  QSettingsBasedStorageManager manager;
  absl::flat_hash_map<std::string, std::filesystem::path> loaded_mappings =
      manager.LoadModuleSymbolFileMappings();

  // Deep compare mappings and loaded_mappings
  EXPECT_EQ(loaded_mappings.size(), mappings.size());
  for (const auto& [module_path, symbol_file_path] : mappings) {
    ASSERT_TRUE(loaded_mappings.contains(module_path));
    EXPECT_EQ(mappings[module_path], loaded_mappings[module_path]);
  }
}

}  // namespace orbit_client_symbols