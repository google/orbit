// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "ClientData/ModuleIdentifier.h"
#include "ClientData/ModuleIdentifierProvider.h"
#include "ClientData/ModulePathAndBuildId.h"

namespace orbit_client_data {

TEST(ModuleIdentifierProvider, CanCreateModuleIdentifiers) {
  ModuleIdentifierProvider module_identifier_provider;
  static const std::string kModulePath = "/module/path";
  static const std::string kBuildId = "build_id";
  ModuleIdentifier module_id1 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = kModulePath, .build_id = kBuildId});
  ModuleIdentifier module_id2 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = kModulePath, .build_id = kBuildId});

  EXPECT_EQ(module_id1, module_id2);

  static const std::string kDifferentModulePath = "/path/to/different/module";
  ModuleIdentifier module_id_different_path = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = kDifferentModulePath, .build_id = kBuildId});

  EXPECT_NE(module_id1, module_id_different_path);

  static const std::string kDifferentBuildId = "build_id2";
  ModuleIdentifier module_id_different_build_id = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = kDifferentModulePath, .build_id = kDifferentBuildId});

  EXPECT_NE(module_id1, module_id_different_build_id);
  EXPECT_NE(module_id_different_path, module_id_different_build_id);
}

TEST(ModuleIdentifierProvider, ReturnsNulloptsForUnknownModule) {
  ModuleIdentifierProvider module_identifier_provider;
  static const std::string kUnknownModulePath = "/path/to/module";
  static const std::string kUnknownBuildId = "build_id";

  std::optional<ModuleIdentifier> module_identifier_opt =
      module_identifier_provider.GetModuleIdentifier(
          {.module_path = kUnknownModulePath, .build_id = kUnknownBuildId});
  EXPECT_FALSE(module_identifier_opt.has_value());

  ModuleIdentifierProvider another_module_identifier_provider;
  ModuleIdentifier unknown_module_identifier =
      another_module_identifier_provider.CreateModuleIdentifier(
          {.module_path = kUnknownModulePath, .build_id = kUnknownBuildId});

  EXPECT_EQ(std::nullopt,
            module_identifier_provider.GetModulePathAndBuildId(unknown_module_identifier));
  EXPECT_EQ(std::nullopt,
            module_identifier_provider.GetModulePathAndBuildId(unknown_module_identifier));
}

TEST(ModuleIdentifierProvider, CanCreateMultipleModuleIdentifiersAndReturnInformationForThem) {
  ModuleIdentifierProvider module_identifier_provider;
  static const std::string kModulePath1 = "/path/to/module";
  static const std::string kBuildId1 = "build_id";

  ModuleIdentifier module_identifier1 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = kModulePath1, .build_id = kBuildId1});

  std::optional<ModuleIdentifier> module_identifier1_opt =
      module_identifier_provider.GetModuleIdentifier(
          {.module_path = kModulePath1, .build_id = kBuildId1});
  ASSERT_TRUE(module_identifier1_opt.has_value());
  EXPECT_EQ(module_identifier1, module_identifier1_opt.value());
  static const std::optional<ModulePathAndBuildId> kExpectedModulePathAndBuildId =
      ModulePathAndBuildId{.module_path = kModulePath1, .build_id = kBuildId1};
  EXPECT_EQ(kExpectedModulePathAndBuildId,
            module_identifier_provider.GetModulePathAndBuildId(module_identifier1));
  static const ModulePathAndBuildId kExpectedModulePathAndBuildId1{.module_path = kModulePath1,
                                                                   .build_id = kBuildId1};
  EXPECT_EQ(kExpectedModulePathAndBuildId1,
            module_identifier_provider.GetModulePathAndBuildId(module_identifier1));

  static const std::string kModulePath2 = "/path/to/another/module";
  static const std::string kBuildId2 = "another_build_id";

  ModuleIdentifier module_identifier2 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = kModulePath2, .build_id = kBuildId2});

  EXPECT_NE(module_identifier1, module_identifier2);
  EXPECT_NE(module_identifier_provider.GetModuleIdentifier(
                {.module_path = kModulePath1, .build_id = kBuildId1}),
            module_identifier_provider.GetModuleIdentifier(
                {.module_path = kModulePath2, .build_id = kBuildId2}));
  EXPECT_NE(module_identifier_provider.GetModulePathAndBuildId(module_identifier1),
            module_identifier_provider.GetModulePathAndBuildId(module_identifier2));

  std::optional<ModuleIdentifier> module_identifier2_opt =
      module_identifier_provider.GetModuleIdentifier(
          {.module_path = kModulePath2, .build_id = kBuildId2});
  ASSERT_TRUE(module_identifier2_opt.has_value());
  EXPECT_EQ(module_identifier2, module_identifier2_opt.value());
  static const ModulePathAndBuildId kExpectedModulePathAndBuildId2{.module_path = kModulePath2,
                                                                   .build_id = kBuildId2};
  EXPECT_EQ(kExpectedModulePathAndBuildId2,
            module_identifier_provider.GetModulePathAndBuildId(module_identifier2));
}

}  // namespace orbit_client_data