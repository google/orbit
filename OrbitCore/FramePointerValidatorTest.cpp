// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <vector>

#include "ElfFile.h"
#include "FramePointerValidator.h"
#include "OrbitFunction.h"
#include "Path.h"

TEST(FramePointerValidator, GetFpoFunctions) {
  std::string executable_path = Path::GetExecutablePath();
  std::string test_elf_file = executable_path + "/testdata/hello_world_elf";

  auto elf_file = ElfFile::Create(test_elf_file);
  ASSERT_NE(elf_file, nullptr);

  Pdb pdb;
  ASSERT_TRUE(elf_file->LoadFunctions(&pdb));
  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  std::optional<std::vector<std::shared_ptr<Function>>> fpo_functions =
      FramePointerValidator::GetFpoFunctions(functions, test_elf_file, true);

  ASSERT_TRUE(fpo_functions.has_value());

  std::vector<std::string> fpo_function_names;

  std::transform(fpo_functions->begin(), fpo_functions->end(),
                 std::back_inserter(fpo_function_names),
                 [](const std::shared_ptr<Function>& f) -> std::string {
                   return f->PrettyName();
                 });

  EXPECT_THAT(fpo_function_names, testing::UnorderedElementsAre(
                                      "_start", "main", "__libc_csu_init"));
}
