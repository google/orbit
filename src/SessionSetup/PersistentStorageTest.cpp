// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>
#include <optional>

#include "OrbitGgp/Client.h"
#include "OrbitGgp/Project.h"
#include "SessionSetup/PersistentStorage.h"

namespace {
constexpr const char* kOrganizationName = "The Orbit Authors";
constexpr const char* kApplicationName{"SessionSetupPersistentStorageTest"};
}  // namespace

namespace orbit_session_setup {

using orbit_ggp::Project;
using InstanceListScope = orbit_ggp::Client::InstanceListScope;

class SessionSetupPersistentStorageTest : public testing::Test {
 protected:
  void SetUp() override {
    QCoreApplication::setOrganizationName(kOrganizationName);
    QCoreApplication::setApplicationName(kApplicationName);

    QSettings settings;
    settings.clear();
  }
};

TEST_F(SessionSetupPersistentStorageTest, SaveAndLoadProject) {
  EXPECT_EQ(LoadLastSelectedProjectFromPersistentStorage(), std::nullopt);  // default is nullopt

  SaveProjectToPersistentStorage(std::nullopt);
  EXPECT_EQ(LoadLastSelectedProjectFromPersistentStorage(), std::nullopt);

  std::optional<Project> project = Project{
      "Test Project Name", /*display_name*/
      "test_project_id",   /*id*/
  };

  SaveProjectToPersistentStorage(project);
  EXPECT_EQ(LoadLastSelectedProjectFromPersistentStorage(), project);
}

TEST_F(SessionSetupPersistentStorageTest, SaveAndLoadInstancesScope) {
  EXPECT_EQ(LoadInstancesScopeFromPersistentStorage(), InstanceListScope::kOnlyOwnInstances);

  SaveInstancesScopeToPersistentStorage(InstanceListScope::kAllReservedInstances);
  EXPECT_EQ(LoadInstancesScopeFromPersistentStorage(), InstanceListScope::kAllReservedInstances);

  SaveInstancesScopeToPersistentStorage(InstanceListScope::kOnlyOwnInstances);
  EXPECT_EQ(LoadInstancesScopeFromPersistentStorage(), InstanceListScope::kOnlyOwnInstances);
}

}  // namespace orbit_session_setup