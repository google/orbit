/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>

#include <unwindstack/PeCoffInterface.h>
#include "PeCoffRuntimeFunctions.h"
#include "unwindstack/Error.h"
#include "utils/MemoryFake.h"

#include <gtest/gtest.h>

namespace unwindstack {

class PeCoffRuntimeFunctionsTest : public ::testing::Test {
 public:
  PeCoffRuntimeFunctionsTest() : memory_fake_(new MemoryFake) {}
  ~PeCoffRuntimeFunctionsTest() {}

  uint64_t SetRuntimeFunctionAtOffset(uint64_t offset, uint32_t start, uint32_t end,
                                      uint32_t unwind_info_offset) {
    memory_fake_->SetData32(offset, start);
    offset += sizeof(uint32_t);
    memory_fake_->SetData32(offset, end);
    offset += sizeof(uint32_t);
    memory_fake_->SetData32(offset, unwind_info_offset);
    offset += sizeof(uint32_t);
    return offset;
  }

  MemoryFake* GetMemoryFake() { return memory_fake_.get(); }

 private:
  std::unique_ptr<MemoryFake> memory_fake_;
};

TEST_F(PeCoffRuntimeFunctionsTest, init_succeeds_on_well_formed_data) {
  uint64_t offset = 0x5000;
  offset = SetRuntimeFunctionAtOffset(offset, 0x100, 0x200, 0x6000);
  offset = SetRuntimeFunctionAtOffset(offset, 0x200, 0x300, 0x6100);
  offset = SetRuntimeFunctionAtOffset(offset, 0x300, 0x400, 0x6100);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions =
      CreatePeCoffRuntimeFunctions(GetMemoryFake());
  EXPECT_TRUE(runtime_functions->Init(0x5000, 0x5000 + 3 * 3 * sizeof(uint32_t)));
}

TEST_F(PeCoffRuntimeFunctionsTest, init_fails_due_to_large_pdata_end_value) {
  uint64_t offset = 0x5000;
  offset = SetRuntimeFunctionAtOffset(offset, 0x100, 0x200, 0x6000);
  offset = SetRuntimeFunctionAtOffset(offset, 0x200, 0x300, 0x6100);
  offset = SetRuntimeFunctionAtOffset(offset, 0x300, 0x400, 0x6100);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions =
      CreatePeCoffRuntimeFunctions(GetMemoryFake());
  // This should fail as the bound of 0x6000 for pdata_end is too large.
  EXPECT_FALSE(runtime_functions->Init(0x5000, 0x6000));
  EXPECT_EQ(ERROR_INVALID_COFF, runtime_functions->GetLastError().code);
}

TEST_F(PeCoffRuntimeFunctionsTest, init_fails_due_to_bad_section_bounds) {
  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions(
      CreatePeCoffRuntimeFunctions(GetMemoryFake()));
  EXPECT_FALSE(runtime_functions->Init(0x5000, 0x4000));
  EXPECT_EQ(ERROR_INVALID_COFF, runtime_functions->GetLastError().code);
}

TEST_F(PeCoffRuntimeFunctionsTest, init_fails_due_to_incongruent_section_bounds) {
  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions =
      CreatePeCoffRuntimeFunctions(GetMemoryFake());
  EXPECT_FALSE(runtime_functions->Init(0x5000, 0x5004));
  EXPECT_EQ(ERROR_INVALID_COFF, runtime_functions->GetLastError().code);
}

TEST_F(PeCoffRuntimeFunctionsTest, init_fails_due_to_bad_memory) {
  constexpr uint64_t kOffset = 0x5000;
  SetRuntimeFunctionAtOffset(kOffset, 0x100, 0x200, 0x6000);
  // Clear the first byte so that reading the memory fails.
  GetMemoryFake()->ClearMemory(kOffset, 1);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions(
      CreatePeCoffRuntimeFunctions(GetMemoryFake()));
  EXPECT_FALSE(runtime_functions->Init(kOffset, kOffset + 3 * sizeof(uint32_t)));
  EXPECT_EQ(ERROR_MEMORY_INVALID, runtime_functions->GetLastError().code);
  EXPECT_EQ(kOffset, runtime_functions->GetLastError().address);
}

TEST_F(PeCoffRuntimeFunctionsTest, find_function_at_the_start) {
  uint64_t offset = 0x5000;
  offset = SetRuntimeFunctionAtOffset(offset, 0x100, 0x200, 0x6000);
  offset = SetRuntimeFunctionAtOffset(offset, 0x200, 0x300, 0x6100);
  offset = SetRuntimeFunctionAtOffset(offset, 0x300, 0x400, 0x6200);
  offset = SetRuntimeFunctionAtOffset(offset, 0x400, 0x500, 0x6300);
  offset = SetRuntimeFunctionAtOffset(offset, 0x500, 0x600, 0x6400);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions(
      CreatePeCoffRuntimeFunctions(GetMemoryFake()));
  ASSERT_TRUE(runtime_functions->Init(0x5000, 0x5000 + 5 * 3 * sizeof(uint32_t)));

  RuntimeFunction function;
  EXPECT_TRUE(runtime_functions->FindRuntimeFunction(0x112, &function));
  EXPECT_EQ(0x100, function.start_address);
  EXPECT_EQ(0x200, function.end_address);
  EXPECT_EQ(0x6000, function.unwind_info_offset);
}

TEST_F(PeCoffRuntimeFunctionsTest, find_function_in_the_middle) {
  uint64_t offset = 0x5000;
  offset = SetRuntimeFunctionAtOffset(offset, 0x100, 0x200, 0x6000);
  offset = SetRuntimeFunctionAtOffset(offset, 0x200, 0x300, 0x6100);
  offset = SetRuntimeFunctionAtOffset(offset, 0x300, 0x400, 0x6200);
  offset = SetRuntimeFunctionAtOffset(offset, 0x400, 0x500, 0x6300);
  offset = SetRuntimeFunctionAtOffset(offset, 0x500, 0x600, 0x6400);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions(
      CreatePeCoffRuntimeFunctions(GetMemoryFake()));
  ASSERT_TRUE(runtime_functions->Init(0x5000, 0x5000 + 5 * 3 * sizeof(uint32_t)));

  RuntimeFunction function;
  EXPECT_TRUE(runtime_functions->FindRuntimeFunction(0x304, &function));
  EXPECT_EQ(0x300, function.start_address);
  EXPECT_EQ(0x400, function.end_address);
  EXPECT_EQ(0x6200, function.unwind_info_offset);
}

TEST_F(PeCoffRuntimeFunctionsTest, find_function_at_the_end) {
  uint64_t offset = 0x5000;
  offset = SetRuntimeFunctionAtOffset(offset, 0x100, 0x200, 0x6000);
  offset = SetRuntimeFunctionAtOffset(offset, 0x200, 0x300, 0x6100);
  offset = SetRuntimeFunctionAtOffset(offset, 0x300, 0x400, 0x6200);
  offset = SetRuntimeFunctionAtOffset(offset, 0x400, 0x500, 0x6300);
  offset = SetRuntimeFunctionAtOffset(offset, 0x500, 0x600, 0x6400);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions(
      CreatePeCoffRuntimeFunctions(GetMemoryFake()));
  ASSERT_TRUE(runtime_functions->Init(0x5000, 0x5000 + 5 * 3 * sizeof(uint32_t)));

  RuntimeFunction function;
  EXPECT_TRUE(runtime_functions->FindRuntimeFunction(0x520, &function));
  EXPECT_EQ(0x500, function.start_address);
  EXPECT_EQ(0x600, function.end_address);
  EXPECT_EQ(0x6400, function.unwind_info_offset);
}

TEST_F(PeCoffRuntimeFunctionsTest, fails_to_find_function_when_address_too_large) {
  uint64_t offset = 0x5000;
  offset = SetRuntimeFunctionAtOffset(offset, 0x100, 0x200, 0x6000);
  offset = SetRuntimeFunctionAtOffset(offset, 0x200, 0x300, 0x6100);
  offset = SetRuntimeFunctionAtOffset(offset, 0x300, 0x400, 0x6200);
  offset = SetRuntimeFunctionAtOffset(offset, 0x400, 0x500, 0x6300);
  offset = SetRuntimeFunctionAtOffset(offset, 0x500, 0x600, 0x6400);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions(
      CreatePeCoffRuntimeFunctions(GetMemoryFake()));
  ASSERT_TRUE(runtime_functions->Init(0x5000, 0x5000 + 5 * 3 * sizeof(uint32_t)));

  RuntimeFunction function;
  EXPECT_FALSE(runtime_functions->FindRuntimeFunction(0x608, &function));
}

TEST_F(PeCoffRuntimeFunctionsTest, fails_to_find_function_when_address_too_small) {
  uint64_t offset = 0x5000;
  offset = SetRuntimeFunctionAtOffset(offset, 0x100, 0x200, 0x6000);
  offset = SetRuntimeFunctionAtOffset(offset, 0x200, 0x300, 0x6100);
  offset = SetRuntimeFunctionAtOffset(offset, 0x300, 0x400, 0x6200);
  offset = SetRuntimeFunctionAtOffset(offset, 0x400, 0x500, 0x6300);
  offset = SetRuntimeFunctionAtOffset(offset, 0x500, 0x600, 0x6400);

  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions(
      CreatePeCoffRuntimeFunctions(GetMemoryFake()));
  ASSERT_TRUE(runtime_functions->Init(0x5000, 0x5000 + 5 * 3 * sizeof(uint32_t)));

  RuntimeFunction function;
  EXPECT_FALSE(runtime_functions->FindRuntimeFunction(0x20, &function));
}
}  // namespace unwindstack