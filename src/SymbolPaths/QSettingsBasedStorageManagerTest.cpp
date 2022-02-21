// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>
#include <filesystem>
#include <memory>

#include "SymbolPaths/QSettingsBasedStorageManager.h"

const std::filesystem::path path0{"/path/to/symbols/path"};
const std::filesystem::path path1{"/home/src/project/build/"};
const std::filesystem::path path2{R"(c:\project\build\)"};

constexpr const char* kOrgName = "The Orbit Authors";

namespace orbit_symbol_paths {

TEST(QSettingsBasedStorageManager, LoadAndSave) {
  QCoreApplication::setOrganizationDomain(kOrgName);
  QCoreApplication::setApplicationName("SymbolPathsManager.SetAndGet");

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

}  // namespace orbit_symbol_paths