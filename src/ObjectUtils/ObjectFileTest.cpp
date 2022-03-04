// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ObjectUtils/ObjectFile.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

using ::orbit_object_utils::CreateObjectFile;
using ::orbit_test_utils::HasNoError;

// Only tests methods that are in the interface for ObjectFile itself. More detailed tests
// specific to ElfFile and CoffFile are in their own tests.

TEST(ObjectFile, CorrectObjectTypeForElf) {
  std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_with_debug_info";

  auto object_file = CreateObjectFile(file_path);
  ASSERT_THAT(object_file, HasNoError());

  EXPECT_TRUE(object_file.value()->IsElf());
  EXPECT_FALSE(object_file.value()->IsCoff());
}

TEST(ObjectFile, CorrectObjectTypeForCoff) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto object_file = CreateObjectFile(file_path);
  ASSERT_THAT(object_file, HasNoError());

  EXPECT_TRUE(object_file.value()->IsCoff());
  EXPECT_FALSE(object_file.value()->IsElf());
}

TEST(ObjectFile, LoadsCoffFileWithSymbols) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto object_file = CreateObjectFile(file_path);
  ASSERT_THAT(object_file, HasNoError());

  EXPECT_TRUE(object_file.value()->HasDebugSymbols());
  const auto symbols_result = object_file.value()->LoadDebugSymbolsAsProto();
  ASSERT_TRUE(symbols_result.has_value()) << symbols_result.error().message();
}

TEST(ObjectFile, LoadsElfFileWithSymbols) {
  std::filesystem::path file_path =
      orbit_test::GetTestdataDir() / "hello_world_elf_with_debug_info";

  auto object_file = CreateObjectFile(file_path);
  ASSERT_THAT(object_file, HasNoError());

  EXPECT_TRUE(object_file.value()->HasDebugSymbols());
  const auto symbols_result = object_file.value()->LoadDebugSymbolsAsProto();
  ASSERT_TRUE(symbols_result.has_value()) << symbols_result.error().message();
}

TEST(ObjectFile, LoadsElfFileWithoutSymbols) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "no_symbols_elf";

  auto object_file = CreateObjectFile(file_path);
  ASSERT_THAT(object_file, HasNoError());

  EXPECT_FALSE(object_file.value()->HasDebugSymbols());
}

TEST(ObjectFile, UsesFilenameAsName) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto object_file = CreateObjectFile(file_path);
  ASSERT_THAT(object_file, HasNoError());

  EXPECT_EQ(object_file.value()->GetName(), "libtest.dll");
}

TEST(ObjectFile, UsesSonameAsNameForElfIfSonameIsPresent) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest-1.0.so";

  auto object_file = CreateObjectFile(file_path);
  ASSERT_THAT(object_file, HasNoError());

  EXPECT_EQ(object_file.value()->GetName(), "libtest.so");
}
