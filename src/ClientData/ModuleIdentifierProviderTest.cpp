// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ModuleIdentifier.h"
#include "ClientData/ModuleIdentifierProvider.h"

namespace orbit_client_data {

TEST(ModuleIdentifierProvider, CanCreateModuleIdentifiers) {
  ModuleIdentifierProvider module_identifier_provider;
  constexpr std::string_view kModulePath = "/module/path";
  constexpr std::string_view kBuildId = "build_id";
  ModuleIdentifier module_id1 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = std::string(kModulePath), .build_id = std::string(kBuildId)});
  ModuleIdentifier module_id2 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = std::string(kModulePath), .build_id = std::string(kBuildId)});

  EXPECT_EQ(module_id1, module_id2);

  constexpr std::string_view kDifferentModulePath = "/path/to/different/module";
  ModuleIdentifier module_id_different_path = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = std::string(kDifferentModulePath), .build_id = std::string(kBuildId)});

  EXPECT_NE(module_id1, module_id_different_path);

  constexpr std::string_view kDifferentBuildId = "build_id2";
  ModuleIdentifier module_id_different_build_id = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = std::string(kDifferentModulePath),
       .build_id = std::string(kDifferentBuildId)});

  EXPECT_NE(module_id1, module_id_different_build_id);
  EXPECT_NE(module_id_different_path, module_id_different_build_id);
}

TEST(ModuleIdentifierProvider, ReturnsNulloptsForUnknownModule) {
  ModuleIdentifierProvider module_identifier_provider;
  constexpr std::string_view kUnknownModulePath = "/path/to/module";
  constexpr std::string_view kUnknownBuildId = "build_id";

  std::optional<ModuleIdentifier> module_identifier_opt =
      module_identifier_provider.GetModuleIdentifier(
          {.module_path = std::string(kUnknownModulePath),
           .build_id = std::string(kUnknownBuildId)});
  EXPECT_FALSE(module_identifier_opt.has_value());

  ModuleIdentifierProvider another_module_identifier_provider;
  ModuleIdentifier unknown_module_identifier =
      another_module_identifier_provider.CreateModuleIdentifier(
          {.module_path = std::string(kUnknownModulePath),
           .build_id = std::string(kUnknownBuildId)});

  EXPECT_EQ(std::nullopt, module_identifier_provider.GetModulePath(unknown_module_identifier));
  EXPECT_EQ(std::nullopt, module_identifier_provider.GetModuleBuildId(unknown_module_identifier));
  EXPECT_EQ(std::nullopt,
            module_identifier_provider.GetModulePathAndBuildId(unknown_module_identifier));
}

TEST(ModuleIdentifierProvider, CanCreateMultipleModuleIdentifiersAndReturnInformationForThem) {
  ModuleIdentifierProvider module_identifier_provider;
  constexpr std::string_view kModulePath1 = "/path/to/module";
  constexpr std::string_view kBuildId1 = "build_id";

  ModuleIdentifier module_identifier1 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = std::string(kModulePath1), .build_id = std::string(kBuildId1)});

  std::optional<ModuleIdentifier> module_identifier1_opt =
      module_identifier_provider.GetModuleIdentifier(
          {.module_path = std::string(kModulePath1), .build_id = std::string(kBuildId1)});
  ASSERT_TRUE(module_identifier1_opt.has_value());
  EXPECT_EQ(module_identifier1, module_identifier1_opt.value());
  EXPECT_EQ(kModulePath1, module_identifier_provider.GetModulePath(module_identifier1));
  EXPECT_EQ(kBuildId1, module_identifier_provider.GetModuleBuildId(module_identifier1));
  static const orbit_symbol_provider::ModulePathAndBuildId kExpectedModulePathAndBuildId1{
      .module_path = std::string{kModulePath1}, .build_id = std::string{kBuildId1}};
  EXPECT_EQ(kExpectedModulePathAndBuildId1,
            module_identifier_provider.GetModulePathAndBuildId(module_identifier1));

  constexpr std::string_view kModulePath2 = "/path/to/another/module";
  constexpr std::string_view kBuildId2 = "another_build_id";

  ModuleIdentifier module_identifier2 = module_identifier_provider.CreateModuleIdentifier(
      {.module_path = std::string(kModulePath2), .build_id = std::string(kBuildId2)});

  EXPECT_NE(module_identifier1, module_identifier2);
  EXPECT_NE(module_identifier_provider.GetModuleIdentifier(
                {.module_path = std::string(kModulePath2), .build_id = std::string(kBuildId2)}),
            module_identifier_provider.GetModuleIdentifier(
                {.module_path = std::string(kModulePath2), .build_id = std::string(kBuildId2)}));
  EXPECT_NE(module_identifier_provider.GetModulePath(module_identifier1),
            module_identifier_provider.GetModulePath(module_identifier2));
  EXPECT_NE(module_identifier_provider.GetModuleBuildId(module_identifier1),
            module_identifier_provider.GetModuleBuildId(module_identifier2));
  EXPECT_NE(module_identifier_provider.GetModulePathAndBuildId(module_identifier1),
            module_identifier_provider.GetModulePathAndBuildId(module_identifier2));

  std::optional<ModuleIdentifier> module_identifier2_opt =
      module_identifier_provider.GetModuleIdentifier(
          {.module_path = std::string(kModulePath2), .build_id = std::string(kBuildId2)});
  ASSERT_TRUE(module_identifier2_opt.has_value());
  EXPECT_EQ(module_identifier2, module_identifier2_opt.value());
  EXPECT_EQ(kModulePath2, module_identifier_provider.GetModulePath(module_identifier2));
  EXPECT_EQ(kBuildId2, module_identifier_provider.GetModuleBuildId(module_identifier2));
  static const orbit_symbol_provider::ModulePathAndBuildId kExpectedModulePathAndBuildId2{
      .module_path = std::string{kModulePath2}, .build_id = std::string{kBuildId2}};
  EXPECT_EQ(kExpectedModulePathAndBuildId2,
            module_identifier_provider.GetModulePathAndBuildId(module_identifier2));
}

}  // namespace orbit_client_data