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

namespace {

void ExpectContainsFunctionName(
    const std::vector<std::shared_ptr<Function>> functions, std::string name) {
  bool found_name = false;
  for (const auto& function : functions) {
    if (function->PrettyName() == name) {
      found_name = true;
      break;
    }
  }
  EXPECT_TRUE(found_name);
}

TEST(FramePointerValidator, GetFpoFunctions) {
  std::string executable_path = Path::GetExecutablePath();
  std::string test_elf_file = executable_path + "/testdata/hello_world_elf";

  auto elf_file = ElfFile::Create(test_elf_file.c_str());
  ASSERT_NE(elf_file, nullptr);

  Pdb pdb;
  ASSERT_TRUE(elf_file->LoadFunctions(&pdb));
  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  std::vector<std::shared_ptr<Function>> fpo_functions =
      FramePointerValidator::GetFpoFunctions(functions, test_elf_file, true);

  EXPECT_EQ(fpo_functions.size(), 6);
  ExpectContainsFunctionName(fpo_functions, "_init");
  ExpectContainsFunctionName(fpo_functions, "_start");
  ExpectContainsFunctionName(fpo_functions, "__do_global_dtors_aux");
  ExpectContainsFunctionName(fpo_functions, "main");
  ExpectContainsFunctionName(fpo_functions, "__libc_csu_init");
  ExpectContainsFunctionName(fpo_functions, "_fini");
}
}  // namespace
