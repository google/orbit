/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <unwindstack/PeCoffInterface.h>

#include <cstring>
#include <limits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <MemoryBuffer.h>
#include "utils/MemoryFake.h"

#include <unwindstack/Log.h>
#include <unwindstack/PeCoffNativeUnwinder.h>

#include "DwarfDebugFrame.h"
#include "PeCoffFake.h"

namespace unwindstack {

template <typename PeCoffInterfaceType>
class PeCoffInterfaceTest : public ::testing::Test {
 public:
  PeCoffInterfaceTest() : fake_(new PeCoffFake<PeCoffInterfaceType>) {}
  ~PeCoffInterfaceTest() override = default;

  PeCoffFake<PeCoffInterfaceType>* GetFake() { return fake_.get(); }
  MemoryFake* GetMemory() { return fake_->GetMemoryFake(); }

 private:
  std::unique_ptr<PeCoffFake<PeCoffInterfaceType>> fake_;
};

// Needed for static_asserts that depend on the template parameter. Just a
// plain `false` in the static_assert will cause compilation to fail.
template <class>
inline constexpr bool kDependentFalse = false;

template <typename PeCoffInterfaceType>
class PeCoffInterfaceFileTest : public ::testing::Test {
 public:
  PeCoffInterfaceFileTest() : memory_(nullptr) {}
  ~PeCoffInterfaceFileTest() override = default;

  static std::unique_ptr<MemoryBuffer> ReadFile(const char* filename) {
    std::string dir = android::base::GetExecutableDirectory() + "/tests/files/";
    std::string data;
    EXPECT_TRUE(android::base::ReadFileToString(dir + filename, &data)) << filename;
    EXPECT_GT(data.size(), 0u);
    auto memory = std::make_unique<MemoryBuffer>();
    EXPECT_TRUE(memory->Resize(data.size()));
    std::memcpy(memory->GetPtr(0), data.data(), data.size());
    return memory;
  }

  void Init() {
    if constexpr (sizeof(typename PeCoffInterfaceType::AddressType) == 8) {
      memory_ = ReadFile("libtest.dll");
      expected_load_bias_ = 0x62640000;
    } else if constexpr (sizeof(typename PeCoffInterfaceType::AddressType) == 4) {
      memory_ = ReadFile("libtest32.dll");
      expected_load_bias_ = 0x67b40000;
    } else {
      static_assert(kDependentFalse<PeCoffInterfaceType>, "AddressType size must be 4 or 8 bytes");
    }
  }
  Memory* GetMemory() { return memory_.get(); };
  int64_t expected_load_bias() { return expected_load_bias_; }

 private:
  std::unique_ptr<Memory> memory_;
  int64_t expected_load_bias_;
};

// Many tests equally apply to the 32-bit and the 64-bit case, so we implement these
// as TYPED_TESTs.
typedef ::testing::Types<PeCoffInterface32, PeCoffInterface64> Implementations;
TYPED_TEST_SUITE(PeCoffInterfaceFileTest, Implementations);
TYPED_TEST_SUITE(PeCoffInterfaceTest, Implementations);

TYPED_TEST(PeCoffInterfaceFileTest, init_for_coff_file) {
  this->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));
  EXPECT_EQ(load_bias, this->expected_load_bias());
}

TYPED_TEST(PeCoffInterfaceTest, init_for_coff_file_fake) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));
  EXPECT_EQ(load_bias, PeCoffFake<TypeParam>::kLoadBiasFake);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_empty_memory) {
  MemoryFake empty;
  TypeParam coff(&empty);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_invalid_memory_at_unused_data) {
  this->GetFake()->Init();
  this->GetMemory()->ClearMemory(0x30, 1);
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_invalid_memory_at_new_header_offset) {
  this->GetFake()->Init();
  this->GetMemory()->ClearMemory(0x3c, 1);
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_wrong_magic_number) {
  this->GetFake()->Init();
  // Correct magic number is 0x5a4d
  this->GetMemory()->SetData16(0, 0x5a4c);
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, new_header_parsing_fails_invalid_memory) {
  this->GetFake()->Init();
  this->GetMemory()->ClearMemory(PeCoffFake<TypeParam>::kNewHeaderOffsetValue, 1);
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, new_header_parsing_fails_wrong_pe_signature) {
  this->GetFake()->Init();
  // Correct PE signature is 0x00004550
  this->GetMemory()->SetData32(PeCoffFake<TypeParam>::kNewHeaderOffsetValue, 0x00004551);
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, coff_header_parsing_fails_invalid_memory) {
  this->GetFake()->Init();
  // COFF header starts 4 bytes after the new header.
  constexpr uint16_t kCoffHeaderStart = PeCoffFake<TypeParam>::kNewHeaderOffsetValue + 4;
  this->GetMemory()->ClearMemory(kCoffHeaderStart, 1);
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_wrong_magic_number) {
  this->GetFake()->Init();
  // 0x010b would be a correct choice
  this->GetMemory()->SetData16(this->GetFake()->optional_header_start_offset(), 0x010a);
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_invalid_memory_start) {
  this->GetFake()->Init();

  this->GetMemory()->ClearMemory(this->GetFake()->optional_header_start_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_invalid_memory_end) {
  this->GetFake()->Init();

  this->GetMemory()->ClearMemory(this->GetFake()->optional_header_start_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_incorrect_header_size) {
  this->GetFake()->Init();

  uint16_t correct_header_size;
  this->GetMemory()->Read16(this->GetFake()->optional_header_size_offset(), &correct_header_size);
  this->GetMemory()->SetData16(this->GetFake()->optional_header_size_offset(),
                               correct_header_size + 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_incorrect_num_data_dir_entries) {
  this->GetFake()->Init();

  uint32_t correct_num;
  this->GetMemory()->Read32(this->GetFake()->optional_header_num_data_dirs_offset(), &correct_num);
  this->GetMemory()->SetData32(this->GetFake()->optional_header_num_data_dirs_offset(),
                               correct_num + 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, section_headers_parsing_fails_invalid_memory) {
  this->GetFake()->InitNoSectionHeaders();
  this->GetMemory()->SetData16(this->GetFake()->coff_header_nsects_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest,
           section_headers_parsing_fails_invalid_memory_middle_of_section_header) {
  const uint64_t section_headers_offset = this->GetFake()->InitNoSectionHeaders();
  this->GetFake()->SetSectionHeaderAtOffset(section_headers_offset, ".text");
  this->GetMemory()->SetData16(this->GetFake()->coff_header_nsects_offset(), 1);
  // We want to catch the second failure case, which is after the initial section name string of
  // length kSectionNameInHeaderSize
  this->GetMemory()->ClearMemory(section_headers_offset + kSectionNameInHeaderSize, 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest,
           section_headers_parsing_fails_section_string_name_offset_not_an_integer) {
  const uint64_t section_headers_offset = this->GetFake()->InitNoSectionHeaders();
  this->GetFake()->SetSectionHeaderAtOffset(section_headers_offset, "/abc");
  this->GetMemory()->SetData16(this->GetFake()->coff_header_nsects_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, section_headers_parsing_fails_missing_string_table) {
  uint64_t offset = this->GetFake()->InitNoSectionHeaders();
  // The "/0" indicates that the section name has to be read at offset 0 in the string table,
  // however for this test, the string table is not set up at all, so it must fail.
  offset = this->GetFake()->SetSectionHeaderAtOffset(offset, "/0");
  this->GetMemory()->SetData32(this->GetFake()->coff_header_symoff_offset(), offset);
  this->GetMemory()->SetData16(this->GetFake()->coff_header_nsects_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, section_headers_parsing_fails_no_text_section) {
  uint64_t offset = this->GetFake()->InitNoSectionHeaders();
  offset = this->GetFake()->SetSectionHeaderAtOffset(offset, ".no_text");
  this->GetMemory()->SetData16(this->GetFake()->coff_header_nsects_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.last_error().code);
}

TYPED_TEST(PeCoffInterfaceTest, debug_frame_section_parsed_correctly) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));

  const DwarfFde* dwarf_fde = coff.DebugFrameSection()->GetFdeFromPc(0x2100);
  ASSERT_NE(dwarf_fde, nullptr);
  EXPECT_EQ(0x2100, dwarf_fde->pc_start);
  EXPECT_EQ(0x2500, dwarf_fde->pc_end);

  const DwarfFde* dwarf_fde2 = coff.DebugFrameSection()->GetFdeFromPc(0x10000);
  ASSERT_EQ(dwarf_fde2, nullptr);
}

TYPED_TEST(PeCoffInterfaceTest, gets_correct_relative_pc_given_map_offset) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));

  constexpr uint64_t kAbsolutePc = 0x2200;
  constexpr uint64_t kMapStart = 0x2000;
  ASSERT_TRUE(kAbsolutePc - kMapStart < PeCoffFake<TypeParam>::kTextSectionMemorySize);
  const uint64_t expected_relative_pc = kAbsolutePc - kMapStart +
                                        PeCoffFake<TypeParam>::kLoadBiasFake +
                                        PeCoffFake<TypeParam>::kTextSectionMemoryOffset;
  EXPECT_EQ(expected_relative_pc,
            coff.GetRelPcWithMapOffset(kAbsolutePc, kMapStart,
                                       PeCoffFake<TypeParam>::kTextSectionFileOffset));
}

TYPED_TEST(PeCoffInterfaceTest, gets_zero_as_relative_pc_if_map_offset_outside_of_any_section) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));

  constexpr uint64_t kMapOffset1 = 0;
  constexpr uint64_t kMapStart1 = 0x2000 + kMapOffset1;
  constexpr uint64_t kAbsolutePc1 = kMapStart1 + 0x200;
  EXPECT_EQ(0, coff.GetRelPcWithMapOffset(kAbsolutePc1, kMapStart1, kMapOffset1));

  constexpr uint64_t kMapOffset2 = 0x100000;
  constexpr uint64_t kMapStart2 = 0x2000 + kMapOffset2;
  constexpr uint64_t kAbsolutePc2 = kMapStart2 + 0x200;
  EXPECT_EQ(0, coff.GetRelPcWithMapOffset(kAbsolutePc2, kMapStart2, kMapOffset2));
}

TYPED_TEST(PeCoffInterfaceTest, gets_correct_relative_pc_given_map_rva) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));

  constexpr uint64_t kAbsolutePc = 0x2200;
  constexpr uint64_t kMapStart = 0x2000;
  ASSERT_TRUE(kAbsolutePc - kMapStart < PeCoffFake<TypeParam>::kTextSectionMemorySize);
  const uint64_t expected_relative_pc = kAbsolutePc - kMapStart +
                                        PeCoffFake<TypeParam>::kLoadBiasFake +
                                        PeCoffFake<TypeParam>::kTextSectionMemoryOffset;
  EXPECT_EQ(expected_relative_pc,
            coff.GetRelPcWithMapRva(kAbsolutePc, kMapStart,
                                    PeCoffFake<TypeParam>::kTextSectionMemoryOffset));
}

TYPED_TEST(PeCoffInterfaceTest, gets_correct_text_range) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));

  uint64_t actual_addr;
  uint64_t actual_size;
  bool actual_result = coff.GetTextRange(&actual_addr, &actual_size);
  ASSERT_TRUE(actual_result);
  EXPECT_EQ(actual_addr, PeCoffFake<TypeParam>::kTextSectionMemoryOffset);
  EXPECT_EQ(actual_size, PeCoffFake<TypeParam>::kTextSectionMemorySize);
}

TYPED_TEST(PeCoffInterfaceTest, gets_no_text_range_if_no_text_section) {
  uint64_t offset = this->GetFake()->InitNoSectionHeaders();
  offset = this->GetFake()->SetSectionHeaderAtOffset(offset, ".no_text");
  this->GetMemory()->SetData16(this->GetFake()->coff_header_nsects_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));

  uint64_t actual_addr;
  uint64_t actual_size;
  bool actual_result = coff.GetTextRange(&actual_addr, &actual_size);
  ASSERT_FALSE(actual_result);
}

TYPED_TEST(PeCoffInterfaceTest, get_correct_text_offset_in_file) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));
  EXPECT_EQ(coff.GetTextOffsetInFile(), PeCoffFake<TypeParam>::kTextSectionFileOffset);
}

TYPED_TEST(PeCoffInterfaceTest, gets_zero_text_offset_in_file_if_no_text_section) {
  uint64_t offset = this->GetFake()->InitNoSectionHeaders();
  offset = this->GetFake()->SetSectionHeaderAtOffset(offset, ".no_text", 1, 2, 3, 4);
  this->GetMemory()->SetData16(this->GetFake()->coff_header_nsects_offset(), 1);

  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));

  EXPECT_EQ(coff.GetTextOffsetInFile(), 0);
}

TYPED_TEST(PeCoffInterfaceTest, gets_correct_size_of_image) {
  this->GetFake()->Init();
  TypeParam coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));
  EXPECT_EQ(coff.GetSizeOfImage(), PeCoffFake<TypeParam>::kSizeOfImage);
}

template <typename AddressType>
class PeCoffInterfaceFake : public PeCoffInterfaceImpl<AddressType> {
 public:
  PeCoffInterfaceFake(Memory* memory) : PeCoffInterfaceImpl<AddressType>(memory) {}

  void SetFakeDebugFrameSection(std::unique_ptr<DwarfSection> debug_frame_section) {
    this->debug_frame_ = std::move(debug_frame_section);
  }
  void SetFakeNativeUnwinder(std::unique_ptr<PeCoffNativeUnwinder> native_unwinder) {
    this->native_unwinder_ = std::move(native_unwinder);
  }
};

template <typename AddressType>
class MockDwarfSection : public DwarfDebugFrame<AddressType> {
 public:
  MockDwarfSection(Memory* memory) : DwarfDebugFrame<AddressType>(memory) {}
  ~MockDwarfSection() override = default;
  MOCK_METHOD(bool, Init, (uint64_t, uint64_t, int64_t), (override));
  MOCK_METHOD(bool, Step, (uint64_t, Regs*, Memory*, bool*, bool*), (override));
};

class MockPeCoffNativeUnwinder : public PeCoffNativeUnwinder {
 public:
  MockPeCoffNativeUnwinder() = default;
  ~MockPeCoffNativeUnwinder() override = default;
  MOCK_METHOD(bool, Init, (), (override));
  MOCK_METHOD(bool, Step, (uint64_t, uint64_t, Regs*, Memory*, bool*, bool*), (override));
};

TYPED_TEST(PeCoffInterfaceTest, step_succeeds_when_debug_frame_step_succeeds) {
  this->GetFake()->Init();
  PeCoffInterfaceFake<typename TypeParam::AddressType> fake_coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(fake_coff.Init(&load_bias));

  bool finished = false;
  bool is_signal_frame = false;

  auto mock_debug_frame_section =
      std::make_unique<MockDwarfSection<typename TypeParam::AddressType>>(nullptr);
  EXPECT_CALL(*mock_debug_frame_section,
              Step(0x2000, nullptr, nullptr, &finished, &is_signal_frame))
      .WillOnce(::testing::Return(true));
  fake_coff.SetFakeDebugFrameSection(std::move(mock_debug_frame_section));
  EXPECT_TRUE(fake_coff.Step(0x2000, 0, nullptr, nullptr, &finished, &is_signal_frame));
}

TYPED_TEST(PeCoffInterfaceTest, step_fails_when_debug_frame_and_native_step_fail) {
  this->GetFake()->Init();
  PeCoffInterfaceFake<typename TypeParam::AddressType> fake_coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(fake_coff.Init(&load_bias));

  bool finished = false;
  bool is_signal_frame = false;

  auto mock_debug_frame_section =
      std::make_unique<MockDwarfSection<typename TypeParam::AddressType>>(nullptr);
  EXPECT_CALL(*mock_debug_frame_section,
              Step(0x2000, nullptr, nullptr, &finished, &is_signal_frame))
      .WillOnce(::testing::Return(false));
  fake_coff.SetFakeDebugFrameSection(std::move(mock_debug_frame_section));

  auto mock_native_unwinder = std::make_unique<MockPeCoffNativeUnwinder>();
  EXPECT_CALL(*mock_native_unwinder, Step(0x2000, 0, nullptr, nullptr, &finished, &is_signal_frame))
      .WillOnce(::testing::Return(false));
  fake_coff.SetFakeNativeUnwinder(std::move(mock_native_unwinder));

  EXPECT_FALSE(fake_coff.Step(0x2000, 0, nullptr, nullptr, &finished, &is_signal_frame));
}

TYPED_TEST(PeCoffInterfaceTest, step_fails_when_debug_frame_and_native_unwinder_are_nullptrs) {
  this->GetFake()->Init();
  PeCoffInterfaceFake<typename TypeParam::AddressType> fake_coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(fake_coff.Init(&load_bias));

  fake_coff.SetFakeDebugFrameSection(nullptr);
  fake_coff.SetFakeNativeUnwinder(nullptr);
  bool finished = false;
  bool is_signal_frame = false;
  EXPECT_FALSE(fake_coff.Step(0x2000, 0, nullptr, nullptr, &finished, &is_signal_frame));
}

TYPED_TEST(PeCoffInterfaceTest, step_succeeds_when_native_step_succeeds_and_debug_frame_fails) {
  this->GetFake()->Init();
  PeCoffInterfaceFake<typename TypeParam::AddressType> fake_coff(this->GetMemory());
  int64_t load_bias;
  ASSERT_TRUE(fake_coff.Init(&load_bias));

  bool finished = false;
  bool is_signal_frame = false;

  auto mock_debug_frame_section =
      std::make_unique<MockDwarfSection<typename TypeParam::AddressType>>(nullptr);
  EXPECT_CALL(*mock_debug_frame_section,
              Step(0x2000, nullptr, nullptr, &finished, &is_signal_frame))
      .WillOnce(::testing::Return(false));
  fake_coff.SetFakeDebugFrameSection(std::move(mock_debug_frame_section));

  auto mock_native_unwinder = std::make_unique<MockPeCoffNativeUnwinder>();
  EXPECT_CALL(*mock_native_unwinder, Step(0x2000, 0, nullptr, nullptr, &finished, &is_signal_frame))
      .WillOnce(::testing::Return(true));
  fake_coff.SetFakeNativeUnwinder(std::move(mock_native_unwinder));

  EXPECT_TRUE(fake_coff.Step(0x2000, 0, nullptr, nullptr, &finished, &is_signal_frame));
}

// The remaining tests are not TYPED_TESTs, because they are specific to either the 32-bit or 64-bit
// version of the PE/COFF interface class, such as testing if a missing data offset for the 32-bit
// instance (this data offset does not exist in the 64-bit case), or initializing a 32-bit instance
// with a 64-bit PE/COFF file cause `Init()` to fail.

using PeCoffInterface32Test = PeCoffInterfaceTest<PeCoffInterface32>;
using PeCoffInterface64Test = PeCoffInterfaceTest<PeCoffInterface64>;

TEST_F(PeCoffInterface32Test, init_64_fails_for_coff_32_fake) {
  GetFake()->Init();
  PeCoffInterface64 coff(GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_UNSUPPORTED, coff.last_error().code);
}

TEST_F(PeCoffInterface64Test, init_32_fails_for_coff_64_fake) {
  GetFake()->Init();
  PeCoffInterface32 coff(GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_UNSUPPORTED, coff.last_error().code);
}

TEST_F(PeCoffInterface32Test, optional_header_parsing_fails_invalid_memory_at_data_offset_32_only) {
  GetFake()->Init();
  const uint64_t kDataOffsetAddress = GetFake()->optional_header_start_offset() + 0x0018;
  this->GetMemory()->ClearMemory(kDataOffsetAddress, 1);

  PeCoffInterface32 coff(GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TEST_F(PeCoffInterface32Test, optional_header_parsing_fails_invalid_memory_end_32) {
  GetFake()->Init();
  const uint64_t kAfterDataOffset = GetFake()->optional_header_start_offset() + 0x0018 + 0x0004;
  this->GetMemory()->ClearMemory(kAfterDataOffset, 1);

  PeCoffInterface32 coff(GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TEST_F(PeCoffInterface64Test, optional_header_parsing_fails_invalid_memory_end_64) {
  GetFake()->Init();
  const uint64_t kAfterDataOffset = GetFake()->optional_header_start_offset() + 0x0018;
  this->GetMemory()->ClearMemory(kAfterDataOffset, 1);

  PeCoffInterface64 coff(GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TEST_F(PeCoffInterface32Test, optional_header_parsing_fails_invalid_memory_data_dirs_32) {
  GetFake()->Init();
  const uint64_t kDataDirOffset =
      GetFake()->optional_header_start_offset() + 0x0018 + 0x0004 + 0x0044;
  this->GetMemory()->ClearMemory(kDataDirOffset, 1);

  PeCoffInterface32 coff(GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

TEST_F(PeCoffInterface64Test, optional_header_parsing_fails_invalid_memory_data_dirs_64) {
  GetFake()->Init();
  const uint64_t kDataDirOffset = GetFake()->optional_header_start_offset() + 0x0018 + 0x0058;
  this->GetMemory()->ClearMemory(kDataDirOffset, 1);

  PeCoffInterface64 coff(GetMemory());
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.last_error().code);
}

}  // namespace unwindstack