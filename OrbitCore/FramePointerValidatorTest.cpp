// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <vector>

#include "ElfFile.h"
#include "FramePointerValidator.h"
#include "OrbitBase/Logging.h"
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

  std::vector<CodeBlock> function_infos;

  std::transform(functions.begin(), functions.end(),
                 std::back_inserter(function_infos),
                 [](const std::shared_ptr<Function>& f) -> CodeBlock {
                   CodeBlock result;
                   result.set_offset(f->Offset());
                   result.set_size(f->Size());
                   return result;
                 });

  std::optional<std::vector<CodeBlock>> fpo_functions =
      FramePointerValidator::GetFpoFunctions(function_infos, test_elf_file,
                                             true);

  ASSERT_TRUE(fpo_functions.has_value());

  std::vector<std::string> fpo_function_names;

  // Retrieve the names of all fpo-functions.
  std::transform(fpo_functions->begin(), fpo_functions->end(),
                 std::back_inserter(fpo_function_names),
                 [&functions](const CodeBlock& f_info) -> std::string {
                   // Find the function with that offset to extract the name.
                   auto function_it = std::find_if(
                       functions.begin(), functions.end(),
                       [&f_info](const std::shared_ptr<Function>& f) {
                         return f->Offset() == f_info.offset();
                       });

                   CHECK(function_it != functions.end());
                   return (*function_it)->PrettyName();
                 });

  EXPECT_THAT(fpo_function_names, testing::UnorderedElementsAre(
                                      "_start", "main", "__libc_csu_init"));
}
