// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <filesystem>
#include <string>
#include <vector>

#include "ClientSymbols/PersistentStorageManager.h"
#include "ClientSymbols/QSettingsBasedStorageManager.h"

const std::filesystem::path path0{"/path/to/symbols/path"};
const std::filesystem::path path1{"/home/src/project/build/"};
const std::filesystem::path path2{R"(c:\project\build\)"};

constexpr const char* kOrgName = "The Orbit Authors";

namespace orbit_client_symbols {

class QSettingsBasedStorageManagerTest : public ::testing::Test {
 public:
  QSettingsBasedStorageManagerTest() {
    QCoreApplication::setOrganizationName(kOrgName);

    const testing::TestInfo* test_info = testing::UnitTest::GetInstance()->current_test_info();
    const QString test_suite_name{test_info->test_suite_name()};
    const QString test_name{test_info->name()};
    QCoreApplication::setApplicationName(QString("%1.%2").arg(test_suite_name).arg(test_name));

    // clear before test;
    QSettings settings;
    settings.clear();
  }
};

TEST_F(QSettingsBasedStorageManagerTest, LoadAndSavePaths) {
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

TEST_F(QSettingsBasedStorageManagerTest, LoadAndSaveModuleSymbolFileMappings) {
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

  EXPECT_THAT(loaded_mappings, testing::UnorderedElementsAreArray(mappings));
}

TEST_F(QSettingsBasedStorageManagerTest, LoadAndSaveDisabledModulePaths) {
  absl::flat_hash_set<std::string> paths{"/path/to/module1", "/path/to/module2",
                                         "/other/module/path"};

  {
    QSettingsBasedStorageManager manager;
    EXPECT_TRUE(manager.LoadDisabledModulePaths().empty());
    manager.SaveDisabledModulePaths(paths);
  }

  QSettingsBasedStorageManager manager;
  absl::flat_hash_set<std::string> loaded_paths = manager.LoadDisabledModulePaths();
  EXPECT_THAT(loaded_paths, testing::UnorderedElementsAreArray(paths));

  manager.SaveDisabledModulePaths({});
  EXPECT_TRUE(manager.LoadDisabledModulePaths().empty());
}

}  // namespace orbit_client_symbols