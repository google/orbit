// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "ElfFile.h"
#include "Path.h"

TEST(ElfFile, LoadFunctions) {
  std::string executable_path = Path::GetExecutablePath();
  std::string test_elf_file = executable_path + "/testdata/hello_world_elf";

  auto elf_file = ElfFile::Create(test_elf_file);
  ASSERT_NE(elf_file, nullptr);

  Pdb pdb{0x104D'ADD2E55, 0x104D'B1A5, test_elf_file, test_elf_file};
  ASSERT_TRUE(elf_file->LoadFunctions(&pdb));
  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();
  EXPECT_EQ(functions.size(), 10);

  const Function* function = functions[0].get();
  EXPECT_EQ(function->Name(), "deregister_tm_clones");
  EXPECT_EQ(function->PrettyName(), "deregister_tm_clones");
  EXPECT_EQ(function->Address(), 0x1080);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetLoadedModulePath(), test_elf_file);
  EXPECT_EQ(function->GetLoadedModuleName(), "hello_world_elf");

  function = functions[9].get();
  EXPECT_EQ(function->Name(), "main");
  EXPECT_EQ(function->PrettyName(), "main");
  EXPECT_EQ(function->Address(), 0x1135);
  EXPECT_EQ(function->Size(), 35);
  EXPECT_EQ(function->GetLoadedModulePath(), test_elf_file);
  EXPECT_EQ(function->GetLoadedModuleName(), "hello_world_elf");
}

TEST(ElfFile, IsAddressInTextSection) {
  std::string executable_path = Path::GetExecutablePath();
  std::string test_elf_file = executable_path + "/testdata/hello_world_elf";

  auto elf_file = ElfFile::Create(test_elf_file);
  ASSERT_NE(elf_file, nullptr);

  EXPECT_FALSE(elf_file->IsAddressInTextSection(0x104F));
  EXPECT_TRUE(elf_file->IsAddressInTextSection(0x1050));
  EXPECT_TRUE(elf_file->IsAddressInTextSection(0x11C0));
  EXPECT_FALSE(elf_file->IsAddressInTextSection(0x11C1));
}

TEST(ElfFile, CalculateLoadBias) {
  const std::string executable_path = Path::GetExecutablePath();
  const std::string test_elf_file_dynamic =
      executable_path + "/testdata/hello_world_elf";

  auto elf_file_dynamic = ElfFile::Create(test_elf_file_dynamic);
  ASSERT_NE(elf_file_dynamic, nullptr);
  EXPECT_EQ(elf_file_dynamic->GetLoadBias(), 0x0);

  const std::string test_elf_file_static =
      executable_path + "/testdata/hello_world_static_elf";
  auto elf_file_static = ElfFile::Create(test_elf_file_static);
  ASSERT_NE(elf_file_static, nullptr);
  EXPECT_EQ(elf_file_static->GetLoadBias(), 0x400000);
}

TEST(ElfFile, CalculateLoadBiasNoProgramHeaders) {
  const std::string executable_path = Path::GetExecutablePath();
  const std::string test_elf_file =
      executable_path + "/testdata/hello_world_elf_no_program_headers";
  auto elf_file = ElfFile::Create(test_elf_file);

  ASSERT_NE(elf_file, nullptr);
  EXPECT_FALSE(elf_file->GetLoadBias().has_value());
}

TEST(ElfFile, HasSymtab) {
  std::string executable_path = Path::GetExecutablePath();
  std::string elf_with_symbols_path =
      executable_path + "/testdata/hello_world_elf";
  std::string elf_without_symbols_path =
      executable_path + "/testdata/no_symbols_elf";

  auto elf_with_symbols = ElfFile::Create(elf_with_symbols_path);
  ASSERT_NE(elf_with_symbols, nullptr);

  EXPECT_TRUE(elf_with_symbols->HasSymtab());

  auto elf_without_symbols = ElfFile::Create(elf_without_symbols_path);
  ASSERT_NE(elf_without_symbols, nullptr);

  EXPECT_FALSE(elf_without_symbols->HasSymtab());
}

TEST(ElfFile, GetBuildId) {
  std::string executable_path = Path::GetExecutablePath();
  std::string hello_world_path = executable_path + "/testdata/hello_world_elf";

  auto hello_world = ElfFile::Create(hello_world_path);
  ASSERT_NE(hello_world, nullptr);
  EXPECT_EQ(hello_world->GetBuildId(),
            "d12d54bc5b72ccce54a408bdeda65e2530740ac8");

  std::string elf_without_build_id_path =
      executable_path + "/testdata/hello_world_elf_no_build_id";

  auto elf_without_build_id = ElfFile::Create(elf_without_build_id_path);
  ASSERT_NE(elf_without_build_id, nullptr);
  EXPECT_EQ(elf_without_build_id->GetBuildId(), "");
}

TEST(ElfFile, GetFilePath) {
  std::string executable_path = Path::GetExecutablePath();
  std::string hello_world_path = executable_path + "/testdata/hello_world_elf";

  auto hello_world = ElfFile::Create(hello_world_path);
  ASSERT_NE(hello_world, nullptr);

  EXPECT_EQ(hello_world->GetFilePath(), hello_world_path);
}

TEST(ElfFile, CreateFromBuffer) {
  std::string executable_path = Path::GetExecutablePath();
  std::string test_elf_file = executable_path + "/testdata/hello_world_elf";

  std::ifstream test_elf_stream{test_elf_file, std::ios::binary};
  const std::string buffer{std::istreambuf_iterator<char>{test_elf_stream},
                           std::istreambuf_iterator<char>{}};
  ASSERT_NE(buffer.size(), 0);

  auto elf_file =
      ElfFile::CreateFromBuffer(test_elf_file, buffer.data(), buffer.size());
  ASSERT_NE(elf_file, nullptr);
  EXPECT_EQ(elf_file->GetBuildId(),
            "d12d54bc5b72ccce54a408bdeda65e2530740ac8");
}
