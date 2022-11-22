// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "GrpcProtos/module.pb.h"
#include "SymbolProvider/ModuleIdentifier.h"

namespace orbit_client_data {

using orbit_grpc_protos::ModuleInfo;
using orbit_symbol_provider::ModuleIdentifier;

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

  ModuleManager module_manager;
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());

  {
    const ModuleIdentifier module_id{file_path, build_id};
    const ModuleData* module = module_manager.GetModuleByModuleIdentifier(module_id);
    const ModuleData* mutable_module = module_manager.GetMutableModuleByModuleIdentifier(module_id);
    ASSERT_NE(module, nullptr);
    EXPECT_EQ(module, mutable_module);
    EXPECT_EQ(module->name(), name);
    EXPECT_EQ(module->file_path(), file_path);
    EXPECT_EQ(module->file_size(), file_size);
    EXPECT_EQ(module->build_id(), build_id);
    EXPECT_EQ(module->load_bias(), load_bias);
  }

  {
    const ModuleData* module_invalid_path =
        module_manager.GetModuleByModuleIdentifier(ModuleIdentifier{"wrong/path", build_id});
    EXPECT_EQ(module_invalid_path, nullptr);
  }

  {
    const ModuleData* module_invalid_build_id =
        module_manager.GetModuleByModuleIdentifier(ModuleIdentifier{file_path, "wrong buildid"});
    EXPECT_EQ(module_invalid_build_id, nullptr);
  }
}

TEST(ModuleManager, GetMutableModuleByModuleIdentifier) {
  std::string name = "name of module";
  std::string file_path = "path/of/module";
  uint64_t file_size = 300;
  std::string build_id = "build id 1";
  uint64_t load_bias = 0x400;
  const ModuleIdentifier module_id{file_path, build_id};

  ModuleInfo module_info;
  module_info.set_name(name);
  module_info.set_file_path(file_path);
  module_info.set_file_size(file_size);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);

  ModuleManager module_manager;
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());

  ModuleData* module = module_manager.GetMutableModuleByModuleIdentifier(module_id);
  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), name);
  EXPECT_EQ(module->file_path(), file_path);
  EXPECT_EQ(module->file_size(), file_size);
  EXPECT_EQ(module->build_id(), build_id);
  EXPECT_EQ(module->load_bias(), load_bias);

  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  module->AddSymbols({});
  EXPECT_EQ(module->GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);

  EXPECT_EQ(
      module_manager.GetMutableModuleByModuleIdentifier(ModuleIdentifier{"wrong/path", build_id}),
      nullptr);
  EXPECT_EQ(module_manager.GetMutableModuleByModuleIdentifier(
                ModuleIdentifier{file_path, "wrong build_id"}),
            nullptr);
}

TEST(ModuleManager, GetModuleByModuleInMemoryAndAddress) {
  constexpr const char* kName = "name of module";
  constexpr const char* kFilePath = "path/of/module";
  constexpr uint64_t kFileSize = 300;
  constexpr const char* kBuildId = "build id 1";
  constexpr uint64_t kLoadBias = 0x4000;
  constexpr uint64_t kExecutableSegmentOffset = 0x25;

  ModuleInfo module_info;
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);
  module_info.set_executable_segment_offset(kExecutableSegmentOffset);

  ModuleManager module_manager;
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());

  ModuleInMemory module_in_memory{0x1000, 0x2000, kFilePath, kBuildId};

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
  constexpr const char* kName = "name of module";
  constexpr const char* kFilePath = "path/of/module";
  constexpr const char* kBuildId = "";
  constexpr uint64_t kFileSize = 300;
  constexpr uint64_t kLoadBias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);

  ModuleManager module_manager;
  std::vector<ModuleData*> unloaded_modules = module_manager.AddOrUpdateModules({module_info});
  EXPECT_EQ(module_manager.GetAllModuleData().size(), 1);
  EXPECT_TRUE(unloaded_modules.empty());

  ModuleData* module =
      module_manager.GetMutableModuleByModuleIdentifier(ModuleIdentifier{kFilePath, kBuildId});

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
  constexpr const char* kDifferentBuildId = "different build id";
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

  const ModuleData* different_module = module_manager.GetModuleByModuleIdentifier(
      ModuleIdentifier{different_path, kDifferentBuildId});
  ASSERT_NE(different_module, nullptr);
  EXPECT_EQ(different_module->file_path(), different_path);
  EXPECT_EQ(different_module->file_size(), different_file_size);
}

TEST(ModuleManager, AddOrUpdateNotLoadedModules) {
  constexpr const char* kName = "name of module";
  constexpr const char* kFilePath = "path/of/module";
  constexpr const char* kBuildId = "";
  constexpr uint64_t kFileSize = 300;
  constexpr uint64_t kLoadBias = 0x400;

  ModuleInfo module_info;
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);

  ModuleManager module_manager;
  std::vector<ModuleData*> not_changed_modules =
      module_manager.AddOrUpdateNotLoadedModules({module_info});
  EXPECT_TRUE(not_changed_modules.empty());

  ModuleData* module =
      module_manager.GetMutableModuleByModuleIdentifier(ModuleIdentifier{kFilePath, kBuildId});

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
  constexpr const char* kDifferentBuildId = "different build id";
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

  const ModuleData* different_module = module_manager.GetModuleByModuleIdentifier(
      ModuleIdentifier{different_path, kDifferentBuildId});
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

  ModuleManager module_manager;
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
