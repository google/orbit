// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <vector>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

namespace orbit_client_data {

TEST(ModuleData, Constructor) {
  constexpr const char* kName = "Example Name";
  constexpr const char* kFilePath = "/test/file/path";
  constexpr uint64_t kFileSize = 1000;
  constexpr const char* kBuildId = "test build id";
  constexpr uint64_t kLoadBias = 4000;
  ModuleInfo::ObjectSegment object_segment;
  object_segment.set_offset_in_file(0x200);
  object_segment.set_size_in_file(0x2FFF);
  object_segment.set_address(0x1000);
  object_segment.set_size_in_memory(0x3000);
  constexpr ModuleInfo::ObjectFileType kObjectFileType = ModuleInfo::kElfFile;

  ModuleInfo module_info{};
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);
  *module_info.add_object_segments() = object_segment;
  module_info.set_object_file_type(kObjectFileType);

  ModuleData module{module_info};

  EXPECT_EQ(module.name(), kName);
  EXPECT_EQ(module.file_path(), kFilePath);
  EXPECT_EQ(module.file_size(), kFileSize);
  EXPECT_EQ(module.build_id(), kBuildId);
  EXPECT_EQ(module.load_bias(), kLoadBias);
  EXPECT_EQ(module.object_file_type(), kObjectFileType);
  ASSERT_EQ(module.GetObjectSegments().size(), 1);
  EXPECT_EQ(module.GetObjectSegments()[0].offset_in_file(), object_segment.offset_in_file());
  EXPECT_EQ(module.GetObjectSegments()[0].size_in_file(), object_segment.size_in_file());
  EXPECT_EQ(module.GetObjectSegments()[0].address(), object_segment.address());
  EXPECT_EQ(module.GetObjectSegments()[0].size_in_memory(), object_segment.size_in_memory());
  EXPECT_FALSE(module.AreAtLeastFallbackSymbolsLoaded());
  EXPECT_FALSE(module.AreDebugSymbolsLoaded());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_TRUE(module.GetFunctions().empty());
}

TEST(ModuleData, ConvertFromVirtualAddressToOffsetInFileAndViceVersaElf) {
  ModuleInfo::ObjectSegment object_segment;
  object_segment.set_offset_in_file(0x1000);
  object_segment.set_size_in_file(0x2FFF);
  object_segment.set_address(0x101000);
  object_segment.set_size_in_memory(0x3000);

  ModuleInfo module_info{};
  module_info.set_load_bias(0x100000);
  *module_info.add_object_segments() = object_segment;
  module_info.set_object_file_type(ModuleInfo::kElfFile);

  ModuleData module{module_info};
  EXPECT_EQ(module.ConvertFromVirtualAddressToOffsetInFile(0x101100), 0x1100);
  EXPECT_EQ(module.ConvertFromOffsetInFileToVirtualAddress(0x1100), 0x101100);
}

TEST(ModuleData, ConvertFromVirtualAddressToOffsetInFileAndViceVersaPe) {
  ModuleInfo::ObjectSegment object_segment;
  object_segment.set_offset_in_file(0x200);
  object_segment.set_size_in_file(0x2FFF);
  object_segment.set_address(0x101000);
  object_segment.set_size_in_memory(0x3000);

  ModuleInfo module_info{};
  module_info.set_load_bias(0x100000);
  *module_info.add_object_segments() = object_segment;
  module_info.set_object_file_type(ModuleInfo::kCoffFile);

  ModuleData module{module_info};
  EXPECT_EQ(module.ConvertFromVirtualAddressToOffsetInFile(0x101100), 0x300);
  EXPECT_EQ(module.ConvertFromOffsetInFileToVirtualAddress(0x300), 0x101100);
}

TEST(ModuleData, ConvertFromVirtualAddressToOffsetInFileAndViceVersaPeNoSections) {
  // PE/COFF file with no section information, fall back to ELF computation.
  ModuleInfo module_info{};
  module_info.set_load_bias(0x100000);
  module_info.set_object_file_type(ModuleInfo::kCoffFile);

  ModuleData module{module_info};
  EXPECT_EQ(module.ConvertFromVirtualAddressToOffsetInFile(0x100300), 0x300);
  EXPECT_EQ(module.ConvertFromOffsetInFileToVirtualAddress(0x300), 0x100300);
}

TEST(ModuleData, AddSymbolsAndAddFallbackSymbols) {
  // Set up ModuleData.
  constexpr const char* kBuildId = "build_id";
  constexpr const char* kModuleFilePath = "/test/file/path";
  ModuleInfo module_info{};
  module_info.set_file_path(kModuleFilePath);
  module_info.set_build_id(kBuildId);
  ModuleData module{module_info};

  // Setup ModuleSymbols.
  constexpr const char* kSymbolPrettyName = "pretty name";
  constexpr uint64_t kSymbolAddress = 15;
  constexpr uint64_t kSymbolSize = 12;

  ModuleSymbols module_symbols;
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_demangled_name(kSymbolPrettyName);
  symbol_info->set_address(kSymbolAddress);
  symbol_info->set_size(kSymbolSize);

  // Actual test.
  const auto verify_get_functions = [&]() {
    const FunctionInfo* function = module.GetFunctions()[0];
    EXPECT_EQ(function->pretty_name(), kSymbolPrettyName);
    EXPECT_EQ(function->module_path(), kModuleFilePath);
    EXPECT_EQ(function->module_build_id(), kBuildId);
    EXPECT_EQ(function->address(), kSymbolAddress);
    EXPECT_EQ(function->size(), kSymbolSize);
  };

  module.AddFallbackSymbols(module_symbols);
  EXPECT_TRUE(module.AreAtLeastFallbackSymbolsLoaded());
  EXPECT_FALSE(module.AreDebugSymbolsLoaded());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);
  EXPECT_EQ(module.GetFunctions().size(), 1);
  verify_get_functions();

  EXPECT_DEATH(module.AddFallbackSymbols(module_symbols), "Check failed");

  module.AddSymbols(module_symbols);
  EXPECT_TRUE(module.AreAtLeastFallbackSymbolsLoaded());
  EXPECT_TRUE(module.AreDebugSymbolsLoaded());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 1);
  verify_get_functions();

  EXPECT_DEATH(module.AddFallbackSymbols(module_symbols), "Check failed");
  EXPECT_DEATH(module.AddSymbols(module_symbols), "Check failed");
}

TEST(ModuleData, FindFunctionFromHash) {
  ModuleSymbols symbols;

  SymbolInfo* symbol = symbols.add_symbol_infos();
  symbol->set_demangled_name("demangled name");

  ModuleData module{ModuleInfo{}};
  module.AddSymbols(symbols);
  ASSERT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
  ASSERT_FALSE(module.GetFunctions().empty());

  const FunctionInfo* function = module.GetFunctions()[0];
  uint64_t hash = function->GetPrettyNameHash();

  {
    const FunctionInfo* result = module.FindFunctionFromHash(hash);
    EXPECT_EQ(result, function);
  }

  {
    const FunctionInfo* result = module.FindFunctionFromHash(hash + 1);
    EXPECT_EQ(result, nullptr);
  }
}

TEST(ModuleData, UpdateIfChangedAndUnload) {
  constexpr const char* kName = "Example Name";
  constexpr const char* kFilePath = "/test/file/path";
  constexpr uint64_t kFileSize = 1000;
  constexpr const char* kBuildId = "";
  constexpr uint64_t kLoadBias = 4000;
  constexpr ModuleInfo::ObjectFileType kObjectFileType = ModuleInfo::kElfFile;

  ModuleInfo module_info{};
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);
  module_info.set_object_file_type(kObjectFileType);

  ModuleData module{module_info};
  EXPECT_EQ(module.name(), kName);
  EXPECT_EQ(module.file_path(), kFilePath);
  EXPECT_EQ(module.file_size(), kFileSize);
  EXPECT_EQ(module.build_id(), kBuildId);
  EXPECT_EQ(module.load_bias(), kLoadBias);
  EXPECT_EQ(module.object_file_type(), kObjectFileType);
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 0);

  // Update the module before any symbol has been added.
  module_info.set_name("different name");
  EXPECT_FALSE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.name(), module_info.name());

  module_info.set_file_size(1002);
  EXPECT_FALSE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.file_size(), module_info.file_size());

  module_info.set_load_bias(4010);
  EXPECT_FALSE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.load_bias(), module_info.load_bias());

  // Add fallback symbols, then update the module: the symbols are deleted.
  ModuleSymbols module_symbols;
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_demangled_name("pretty_name");
  symbol_info->set_address(0xadd2355);
  symbol_info->set_size(0x5123);

  module.AddFallbackSymbols(module_symbols);
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);
  EXPECT_EQ(module.GetFunctions().size(), 1);

  module_info.set_file_size(1003);
  EXPECT_TRUE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.file_size(), module_info.file_size());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 0);

  // Add symbols, then update the module: symbols are deleted.
  module.AddSymbols(module_symbols);
  EXPECT_EQ(module.GetFunctions().size(), 1);
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);

  module_info.set_file_size(1004);
  EXPECT_TRUE(module.UpdateIfChangedAndUnload(module_info));
  EXPECT_EQ(module.file_size(), module_info.file_size());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 0);

  // file_path is not allowed to be changed
  module_info.set_file_path("changed/path");
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");

  // as well as build_id
  module_info.set_build_id("yet another build id");
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");

  // and object_file_type
  module_info.set_object_file_type(ModuleInfo::kUnknown);
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");
}

TEST(ModuleData, UpdateIfChangedAndNotLoaded) {
  constexpr const char* kName = "Example Name";
  constexpr const char* kFilePath = "/test/file/path";
  constexpr uint64_t kFileSize = 1000;
  constexpr const char* kBuildId = "";
  constexpr uint64_t kLoadBias = 4000;
  ModuleInfo::ObjectFileType object_file_type = ModuleInfo::kElfFile;

  ModuleInfo module_info{};
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);
  module_info.set_object_file_type(object_file_type);

  ModuleData module{module_info};
  EXPECT_EQ(module.name(), kName);
  EXPECT_EQ(module.file_path(), kFilePath);
  EXPECT_EQ(module.file_size(), kFileSize);
  EXPECT_EQ(module.build_id(), kBuildId);
  EXPECT_EQ(module.load_bias(), kLoadBias);
  EXPECT_EQ(module.object_file_type(), object_file_type);
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 0);

  // Update the module before any symbol has been added.
  module_info.set_name("different name");
  EXPECT_TRUE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_EQ(module.name(), module_info.name());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 0);

  module_info.set_file_size(1002);
  EXPECT_TRUE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_EQ(module.file_size(), module_info.file_size());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 0);

  module_info.set_load_bias(4010);
  EXPECT_TRUE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_EQ(module.load_bias(), module_info.load_bias());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 0);

  // Add fallback symbols, then try to update the module: the module is not updated and the symbols
  // are kept.
  ModuleSymbols module_symbols;
  SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
  symbol_info->set_demangled_name("pretty_name");
  symbol_info->set_address(0xadd2355);
  symbol_info->set_size(0x5123);

  module.AddFallbackSymbols(module_symbols);
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);
  EXPECT_EQ(module.GetFunctions().size(), 1);

  module_info.set_file_size(1003);
  EXPECT_FALSE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_NE(module.file_size(), module_info.file_size());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);
  EXPECT_EQ(module.GetFunctions().size(), 1);

  // Add symbols, then try to update the module: the module is not updated and the symbols are kept.
  module.AddSymbols(module_symbols);
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 1);

  module_info.set_file_size(1003);
  EXPECT_FALSE(module.UpdateIfChangedAndNotLoaded(module_info));
  EXPECT_NE(module.file_size(), module_info.file_size());
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
  EXPECT_EQ(module.GetFunctions().size(), 1);

  // file_path is not allowed to be changed
  module_info.set_file_path("changed/path");
  EXPECT_DEATH((void)module.UpdateIfChangedAndNotLoaded(module_info), "Check failed");

  // as well as build_id
  module_info.set_build_id("yet another build id");
  EXPECT_DEATH((void)module.UpdateIfChangedAndNotLoaded(module_info), "Check failed");

  // and object_file_type
  module_info.set_object_file_type(ModuleInfo::kUnknown);
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");
}

TEST(ModuleData, UpdateIfChangedWithBuildId) {
  constexpr const char* kName = "Example Name";
  constexpr const char* kFilePath = "/test/file/path";
  constexpr uint64_t kFileSize = 1000;
  constexpr const char* kBuildId = "build_id_27";
  constexpr uint64_t kLoadBias = 4000;
  constexpr ModuleInfo::ObjectFileType kObjectFileType = ModuleInfo::kElfFile;

  ModuleInfo module_info{};
  module_info.set_name(kName);
  module_info.set_file_path(kFilePath);
  module_info.set_file_size(kFileSize);
  module_info.set_build_id(kBuildId);
  module_info.set_load_bias(kLoadBias);
  module_info.set_object_file_type(kObjectFileType);

  ModuleData module{module_info};

  EXPECT_EQ(module.name(), kName);
  EXPECT_EQ(module.file_path(), kFilePath);
  EXPECT_EQ(module.file_size(), kFileSize);
  EXPECT_EQ(module.build_id(), kBuildId);
  EXPECT_EQ(module.load_bias(), kLoadBias);
  EXPECT_EQ(module.object_file_type(), kObjectFileType);
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kNoSymbols);
  EXPECT_TRUE(module.GetFunctions().empty());

  // We cannot change a module with non-empty build_id
  module_info.set_name("different name");
  EXPECT_DEATH((void)module.UpdateIfChangedAndUnload(module_info), "Check failed");
  EXPECT_DEATH((void)module.UpdateIfChangedAndNotLoaded(module_info), "Check failed");

  // Adding symbols and fallback symbols should work.
  module.AddFallbackSymbols({});
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(),
            ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo);
  module.AddSymbols({});
  EXPECT_EQ(module.GetLoadedSymbolsCompleteness(), ModuleData::SymbolCompleteness::kDebugSymbols);
}

}  // namespace orbit_client_data
