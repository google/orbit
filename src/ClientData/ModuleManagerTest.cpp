// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <optional>
#include <string>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ModuleIdentifier.h"
#include "ClientData/ModuleIdentifierProvider.h"
#include "ClientData/ModuleInMemory.h"
#include "ClientData/ModuleManager.h"
#include "GrpcProtos/module.pb.h"

namespace orbit_client_data {

using orbit_client_data::ModuleIdentifier;
using orbit_grpc_protos::ModuleInfo;

TEST(ModuleManager, GetModuleByModuleIdentifier) {
  std::string name = "name of module";
  std::string file_path = "path/of/module";
  uint64_t file_size = 300;
  std::string build_id = "build id 1";
  uint64_t load_bias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);

  ModuleIdentifierProvider module_identifier_provider{};
  ModuleManager module_manager{&module_identifier_provider};
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());

  const std::optional<ModuleIdentifier> module_id = module_identifier_provider.GetModuleIdentifier(
      {.module_path = file_path, .build_id = build_id});
  ASSERT_TRUE(module_id.has_value());
  const ModuleData* module = module_manager.GetModuleByModuleIdentifier(module_id.value());
  const ModuleData* mutable_module =
      module_manager.GetMutableModuleByModuleIdentifier(module_id.value());
  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module, mutable_module);
  EXPECT_EQ(module->name(), name);
  EXPECT_EQ(module->file_path(), file_path);
  EXPECT_EQ(module->file_size(), file_size);
  EXPECT_EQ(module->build_id(), build_id);
  EXPECT_EQ(module->load_bias(), load_bias);
}

TEST(ModuleManager, GetMutableModuleByModuleIdentifier) {
  std::string name = "name of module";
  std::string file_path = "path/of/module";
  uint64_t file_size = 300;
  std::string build_id = "build id 1";
  uint64_t load_bias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);

  ModuleIdentifierProvider module_identifier_provider{};
  ModuleManager module_manager{&module_identifier_provider};
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());

  const std::optional<ModuleIdentifier> module_id = module_identifier_provider.GetModuleIdentifier(
      {.module_path = file_path, .build_id = build_id});
  ASSERT_TRUE(module_id.has_value());
  ModuleData* module = module_manager.GetMutableModuleByModuleIdentifier(module_id.value());
  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), name);
  EXPECT_EQ(module->file_path(), file_path);
  EXPECT_EQ(module->file_size(), file_size);
  EXPECT_EQ(module->build_id(), build_id);
  EXPECT_EQ(module->load_bias(), load_bias);

  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  module->AddSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
}

TEST(ModuleManager, GetModuleByModuleInMemoryAndAddress) {
  static const std::string kName = "name of module";
  static const std::string kFilePath = "path/of/module";
  constexpr uint64_t kFileSize = 300;
  static const std::string kBuildId = "build id 1";
  constexpr uint64_t kLoadBias = 0x4000;
  constexpr uint64_t kExecutableSegmentOffset = 0x25;

  ModuleInfo module_info;
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);
  module_info.set_executable_segment_offset(kExecutableSegmentOffset);

  ModuleIdentifierProvider module_identifier_provider{};
  ModuleManager module_manager{&module_identifier_provider};
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());

  std::optional<ModuleIdentifier> module_identifier =
      module_identifier_provider.GetModuleIdentifier(
          {.module_path = kFilePath, .build_id = kBuildId});
  ASSERT_TRUE(module_identifier.has_value());
  ModuleInMemory module_in_memory{0x1000, 0x2000, module_identifier.value()};

  EXPECT_NE(module_manager.GetModuleByModuleIdentifier(module_in_memory.module_id()), nullptr);
  EXPECT_EQ(module_manager.GetModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1000),
            nullptr);
  EXPECT_EQ(module_manager.GetModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1010),
            nullptr);
  EXPECT_EQ(module_manager.GetModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1024),
            nullptr);
  EXPECT_EQ(
      module_manager.GetMutableModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1000),
      nullptr);
  EXPECT_EQ(
      module_manager.GetMutableModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1010),
      nullptr);
  EXPECT_EQ(
      module_manager.GetMutableModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1024),
      nullptr);

  const ModuleData* target_module =
      module_manager.GetModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1025);
  EXPECT_NE(target_module, nullptr);
  EXPECT_EQ(
      module_manager.GetMutableModuleByModuleInMemoryAndAbsoluteAddress(module_in_memory, 0x1025),
      target_module);

  EXPECT_EQ(target_module->name(), kName);
  EXPECT_EQ(target_module->file_path(), kFilePath);
  EXPECT_EQ(target_module->file_size(), kFileSize);
  EXPECT_EQ(target_module->build_id(), kBuildId);
  EXPECT_EQ(target_module->load_bias(), kLoadBias);
  EXPECT_EQ(target_module->executable_segment_offset(), kExecutableSegmentOffset);
}

TEST(ModuleManager, AddOrUpdateModules) {
  static const std::string kName = "name of module";
  static const std::string kFilePath = "path/of/module";
  static const std::string kBuildId = "";
  constexpr uint64_t kFileSize = 300;
  constexpr uint64_t kLoadBias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);

  ModuleIdentifierProvider module_identifier_provider{};
  ModuleManager module_manager{&module_identifier_provider};
  std::vector<ModuleData*> unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);
  EXPECT_TRUE(unloaded_modules.empty());

  ModuleData* module = module_manager.GetMutableModuleByModulePathAndBuildId(
      {.module_path = kFilePath, .build_id = kBuildId});

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), kName);

  // Change the name: this updates the module.
  std::string changed_name = "changed name";
  module_info.set_name(changed_name);

  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);
  EXPECT_TRUE(unloaded_modules.empty());

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->file_path(), kFilePath);
  EXPECT_EQ(module->name(), changed_name);

  // Add fallback symbols.
  module->AddFallbackSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);

  // Update the module again.
  module_info.set_name("changed name 2");
  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);
  ASSERT_EQ(unloaded_modules.size(), 1);
  EXPECT_EQ(unloaded_modules[0]->name(), "changed name 2");
  EXPECT_EQ(unloaded_modules[0]->GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kNoSymbols);

  // Add symbols.
  module->AddSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);

  // Update the module yet again.
  module_info.set_name("changed name 3");
  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);
  ASSERT_EQ(unloaded_modules.size(), 1);
  EXPECT_EQ(unloaded_modules[0]->name(), "changed name 3");
  EXPECT_EQ(unloaded_modules[0]->GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kNoSymbols);

  // Add symbols again.
  module->AddSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);

  // Change the build id: this creates a new module.
  static const std::string kDifferentBuildId = "different build id";
  module_info.set_build_id(kDifferentBuildId);
  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 2);
  EXPECT_TRUE(unloaded_modules.empty());

  EXPECT_EQ(module->build_id(), kBuildId);
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);

  // Change file path and file size, and add the module again: this creates another new module
  // because of the different file path.
  std::string different_path = "different/path/of/module";
  module_info.set_file_path(different_path);
  uint64_t different_file_size = 301;
  module_info.set_file_size(different_file_size);
  unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 3);
  EXPECT_TRUE(unloaded_modules.empty());

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->file_path(), kFilePath);
  EXPECT_EQ(module->file_size(), kFileSize);

  const ModuleData* different_module = module_manager.GetModuleByModulePathAndBuildId(
      {.module_path = different_path, .build_id = kDifferentBuildId});
  ASSERT_NE(different_module, nullptr);
  EXPECT_EQ(different_module->file_path(), different_path);
  EXPECT_EQ(different_module->file_size(), different_file_size);
}

TEST(ModuleManager, AddOrUpdateNotLoadedModules) {
  static const std::string kName = "name of module";
  static const std::string kFilePath = "path/of/module";
  static const std::string kBuildId = "";
  constexpr uint64_t kFileSize = 300;
  constexpr uint64_t kLoadBias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);

  ModuleIdentifierProvider module_identifier_provider{};
  ModuleManager module_manager{&module_identifier_provider};
  std::vector<ModuleData*> not_changed_modules =
      module_manager.AddOrUpdateNotLoadedModules({module_info});
  EXPECT_TRUE(not_changed_modules.empty());

  ModuleData* module = module_manager.GetMutableModuleByModulePathAndBuildId(
      {.module_path = kFilePath, .build_id = kBuildId});

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), kName);

  // Change the name: this updates the module.
  std::string changed_name = "changed name";
  module_info.set_name(changed_name);

  not_changed_modules = module_manager.AddOrUpdateNotLoadedModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);
  EXPECT_TRUE(not_changed_modules.empty());

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->file_path(), kFilePath);
  EXPECT_EQ(module->name(), changed_name);

  // Add fallback symbols and try to update the module: the module won't be updated.
  module->AddFallbackSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);

  module_info.set_name("changed name 2");
  not_changed_modules = module_manager.AddOrUpdateNotLoadedModules({module_info});
  ASSERT_EQ(not_changed_modules.size(), 1);
  EXPECT_EQ(not_changed_modules[0], module);
  EXPECT_EQ(not_changed_modules[0]->name(), changed_name);
  EXPECT_EQ(not_changed_modules[0]->GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);

  // Add symbols and try to update the module: the module won't be updated.
  module->AddSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);

  module_info.set_name("changed name 2");
  not_changed_modules = module_manager.AddOrUpdateNotLoadedModules({module_info});
  ASSERT_EQ(not_changed_modules.size(), 1);
  EXPECT_EQ(not_changed_modules[0], module);
  EXPECT_EQ(not_changed_modules[0]->name(), changed_name);
  EXPECT_EQ(not_changed_modules[0]->GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDebugSymbols);

  // Change the build id: this creates a new module.
  static const std::string kDifferentBuildId = "different build id";
  module_info.set_build_id(kDifferentBuildId);
  not_changed_modules = module_manager.AddOrUpdateNotLoadedModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 2);
  EXPECT_TRUE(not_changed_modules.empty());

  EXPECT_EQ(module->build_id(), kBuildId);
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);

  // Change file path and file size, and add the module again: this creates another new module
  // because of the different file path.
  std::string different_path = "different/path/of/module";
  module_info.set_file_path(different_path);
  uint64_t different_file_size = 301;
  module_info.set_file_size(different_file_size);
  not_changed_modules = module_manager.AddOrUpdateNotLoadedModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 3);
  EXPECT_TRUE(not_changed_modules.empty());

  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->file_path(), kFilePath);
  EXPECT_EQ(module->file_size(), kFileSize);

  const ModuleData* different_module = module_manager.GetModuleByModulePathAndBuildId(
      {.module_path = different_path, .build_id = kDifferentBuildId});
  ASSERT_NE(different_module, nullptr);
  EXPECT_EQ(different_module->file_path(), different_path);
  EXPECT_EQ(different_module->file_size(), different_file_size);
}

TEST(ModuleManager, GetModulesByFilename) {
  ModuleInfo module_info_1;
  module_info_1.set_name("name_1");
  module_info_1.set_file_path("path/to/file_1.ext");
  module_info_1.set_build_id("build_id_1");

  ModuleInfo module_info_2;
  module_info_2.set_name("name_2");
  module_info_2.set_file_path("path/to/file_2.ext");
  module_info_2.set_build_id("build_id_2");

  ModuleInfo module_info_3;
  module_info_3.set_name("name_3");
  module_info_3.set_file_path("path/to/file_3.ext");
  module_info_3.set_build_id("build_id_3");

  ModuleIdentifierProvider module_identifier_provider{};
  ModuleManager module_manager{&module_identifier_provider};
  std::vector<ModuleData*> changed_modules =
      module_manager.AddOrUpdateModules({module_info_1, module_info_2, module_info_3});
  EXPECT_TRUE(changed_modules.empty());

  {  // Empty
    std::vector<const ModuleData*> found_modules = module_manager.GetModulesByFilename("");
    EXPECT_TRUE(found_modules.empty());
  }

  {  // Not existing filename
    std::vector<const ModuleData*> found_modules =
        module_manager.GetModulesByFilename("notExistingFilename.ext");
    EXPECT_TRUE(found_modules.empty());
  }

  {  // filename without extension
    std::vector<const ModuleData*> found_modules = module_manager.GetModulesByFilename("file_2");
    EXPECT_TRUE(found_modules.empty());
  }

  {  // find file_2.ext
    std::vector<const ModuleData*> found_modules =
        module_manager.GetModulesByFilename("file_2.ext");
    ASSERT_EQ(found_modules.size(), 1);
    const ModuleData* found_module = found_modules[0];
    EXPECT_EQ(found_module->name(), module_info_2.name());
    EXPECT_EQ(found_module->file_path(), module_info_2.file_path());
    EXPECT_EQ(found_module->build_id(), module_info_2.build_id());
  }
}

}  // namespace orbit_client_data
