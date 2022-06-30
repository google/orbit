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

#include <unwindstack/PeCoff.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unwindstack/MapInfo.h>
#include <unwindstack/Memory.h>
#include <unwindstack/PeCoffInterface.h>
#include <unwindstack/Regs.h>

#include "MemoryFake.h"
#include "PeCoffFake.h"

namespace unwindstack {

class MockPeCoffInterface : public PeCoffInterface {
 public:
  MockPeCoffInterface() = default;
  MOCK_METHOD(bool, Init, (int64_t*), (override));
  MOCK_METHOD(const ErrorData&, last_error, (), (override));
  MOCK_METHOD(ErrorCode, LastErrorCode, (), (override));
  MOCK_METHOD(uint64_t, LastErrorAddress, (), (override));
  MOCK_METHOD(DwarfSection*, DebugFrameSection, (), (override));
  MOCK_METHOD(uint64_t, GetRelPcWithMapOffset, (uint64_t, uint64_t, uint64_t), (override));
  MOCK_METHOD(uint64_t, GetRelPcWithMapRva, (uint64_t, uint64_t, uint64_t), (override));
  MOCK_METHOD(bool, GetTextRange, (uint64_t*, uint64_t*), (const override));
  MOCK_METHOD(uint64_t, GetTextOffsetInFile, (), (const override));
  MOCK_METHOD(uint64_t, GetSizeOfImage, (), (const override));
  MOCK_METHOD(bool, Step, (uint64_t, uint64_t, Regs*, Memory*, bool*, bool*), (override));
};

class FakePeCoff : public PeCoff {
 public:
  FakePeCoff(Memory* memory) : PeCoff(memory) {}
  void SetFakePeCoffInterface(PeCoffInterface* pe_coff_interface) {
    this->interface_.reset(pe_coff_interface);
  }
};

template <typename PeCoffInterfaceType>
class PeCoffTest : public ::testing::Test {
 public:
  PeCoffTest() : fake_(new PeCoffFake<PeCoffInterfaceType>) {}
  ~PeCoffTest() override = default;

  PeCoffFake<PeCoffInterfaceType>* GetFake() { return fake_.get(); }
  MemoryFake* ReleaseMemory() { return fake_->ReleaseMemoryFake(); }

 private:
  std::unique_ptr<PeCoffFake<PeCoffInterfaceType>> fake_;
};

// Many tests equally apply to the 32-bit and the 64-bit case, so we implement these
// as TYPED_TESTs.
typedef ::testing::Types<PeCoffInterface32, PeCoffInterface64> Implementations;
TYPED_TEST_SUITE(PeCoffTest, Implementations);

TYPED_TEST(PeCoffTest, init_succeeds_on_well_formed_file) {
  this->GetFake()->Init();
  PeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_TRUE(coff.valid());
}

TYPED_TEST(PeCoffTest, init_fails_on_bad_file) {
  PeCoff coff(new MemoryFake);
  EXPECT_FALSE(coff.Init());
  EXPECT_FALSE(coff.valid());
}

TYPED_TEST(PeCoffTest, invalidate_works_on_valid_object) {
  this->GetFake()->Init();
  PeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_TRUE(coff.valid());
  coff.Invalidate();
  EXPECT_FALSE(coff.valid());
}

TYPED_TEST(PeCoffTest, returns_correct_load_bias_after_init_succeeds) {
  this->GetFake()->Init();
  PeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_EQ(PeCoffFake<TypeParam>::kLoadBiasFake, coff.GetLoadBias());
}

TYPED_TEST(PeCoffTest, retuns_zero_as_load_bias_on_invalid_object) {
  PeCoff coff(new MemoryFake);
  EXPECT_FALSE(coff.Init());
  EXPECT_FALSE(coff.valid());
  EXPECT_EQ(0, coff.GetLoadBias());
}

TYPED_TEST(PeCoffTest, getting_build_id_aborts) {
  this->GetFake()->Init();
  PeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  ASSERT_DEATH(coff.GetBuildID(), "");
}

TYPED_TEST(PeCoffTest, getting_soname_aborts) {
  this->GetFake()->Init();
  PeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  ASSERT_DEATH(coff.GetSoname(), "");
}

TYPED_TEST(PeCoffTest, getting_function_name_fails) {
  this->GetFake()->Init();
  PeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_FALSE(coff.GetFunctionName(0, nullptr, nullptr));
}

TYPED_TEST(PeCoffTest, getting_global_variable_offset_aborts) {
  this->GetFake()->Init();
  PeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  ASSERT_DEATH(coff.GetGlobalVariableOffset("", nullptr), "");
}

TYPED_TEST(PeCoffTest, rel_pc_is_computed_using_offset_and_correctly_passed_through) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());

  constexpr uint64_t kPcValue = 0x2000;
  constexpr uint64_t kMapStart = 0x1000;
  constexpr uint64_t kMapEnd = 0x4000;
  constexpr uint64_t kMapObjectOffset = 0x200;

  // This test is not testing whether the GetRelPc computation is correct, only whether the
  // return value from PeCoffInterface::GetRelPc is correctly passed through.
  constexpr uint64_t kMockRelPc = 0x3000;
  MockPeCoffInterface* mock_interface = new MockPeCoffInterface;
  EXPECT_CALL(*mock_interface, GetRelPcWithMapOffset(kPcValue, kMapStart, kMapObjectOffset))
      .WillOnce(::testing::Return(kMockRelPc));
  EXPECT_CALL(*mock_interface, GetRelPcWithMapRva).Times(0);
  coff.SetFakePeCoffInterface(mock_interface);

  auto map_info = MapInfo::Create(kMapStart, kMapEnd, 0, 0, "no_name");
  map_info->set_object_offset(kMapObjectOffset);
  EXPECT_EQ(kMockRelPc, coff.GetRelPc(kPcValue, map_info.get()));
}

TYPED_TEST(PeCoffTest, rel_pc_is_computed_using_rva_and_correctly_passed_through) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());

  constexpr uint64_t kPcValue = 0x2000;
  constexpr uint64_t kMapStart = 0x1000;
  constexpr uint64_t kMapEnd = 0x4000;
  constexpr uint64_t kMapObjectRva = 0x3000;

  constexpr uint64_t kMockRelPc = 0x3000;
  MockPeCoffInterface* mock_interface = new MockPeCoffInterface;
  EXPECT_CALL(*mock_interface, GetRelPcWithMapOffset).Times(0);
  EXPECT_CALL(*mock_interface, GetRelPcWithMapRva(kPcValue, kMapStart, kMapObjectRva))
      .WillOnce(::testing::Return(kMockRelPc));
  coff.SetFakePeCoffInterface(mock_interface);

  auto map_info = MapInfo::Create(kMapStart, kMapEnd, 0, 0, "no_name");
  map_info->set_object_rva(kMapObjectRva);
  EXPECT_EQ(kMockRelPc, coff.GetRelPc(kPcValue, map_info.get()));
}

TYPED_TEST(PeCoffTest, rel_pc_is_zero_for_invalid) {
  PeCoff coff(new MemoryFake);
  EXPECT_FALSE(coff.Init());
  EXPECT_FALSE(coff.valid());
  EXPECT_EQ(0, coff.GetRelPc(0, nullptr));
}

TYPED_TEST(PeCoffTest, text_range_is_correctly_passed_through_and_adjusted_by_image_base) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());

  constexpr uint64_t kAddr = 0x1000;
  constexpr uint64_t kSize = 0x2000;

  auto* mock_interface = new MockPeCoffInterface;
  EXPECT_CALL(*mock_interface, GetTextRange).WillOnce([](uint64_t* addr, uint64_t* size) {
    *addr = kAddr;
    *size = kSize;
    return true;
  });
  coff.SetFakePeCoffInterface(mock_interface);

  uint64_t actual_addr;
  uint64_t actual_size;
  bool actual_result = coff.GetTextRange(&actual_addr, &actual_size);
  ASSERT_TRUE(actual_result);
  EXPECT_EQ(actual_addr, PeCoffFake<TypeParam>::kLoadBiasFake + kAddr);
  EXPECT_EQ(actual_size, kSize);
}

TYPED_TEST(PeCoffTest, no_text_range_for_invalid) {
  PeCoff coff(new MemoryFake);
  EXPECT_FALSE(coff.Init());
  EXPECT_FALSE(coff.valid());
  uint64_t actual_addr;
  uint64_t actual_size;
  EXPECT_FALSE(coff.GetTextRange(&actual_addr, &actual_size));
}

TYPED_TEST(PeCoffTest, text_offset_in_file_is_correctly_passed_through) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());

  constexpr uint64_t kSize = 0x2000;

  auto* mock_interface = new MockPeCoffInterface;
  EXPECT_CALL(*mock_interface, GetTextOffsetInFile).WillOnce(testing::Return(kSize));
  coff.SetFakePeCoffInterface(mock_interface);

  EXPECT_EQ(coff.GetTextOffsetInFile(), kSize);
}

TYPED_TEST(PeCoffTest, zero_text_offset_in_file_for_invalid) {
  PeCoff coff(new MemoryFake);
  EXPECT_FALSE(coff.Init());
  EXPECT_FALSE(coff.valid());
  EXPECT_EQ(coff.GetTextOffsetInFile(), 0);
}

TYPED_TEST(PeCoffTest, size_of_image_is_correctly_passed_through) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());

  constexpr uint64_t kSizeOfImage = 0x2000;

  auto* mock_interface = new MockPeCoffInterface;
  EXPECT_CALL(*mock_interface, GetSizeOfImage).WillOnce(testing::Return(kSizeOfImage));
  coff.SetFakePeCoffInterface(mock_interface);

  EXPECT_EQ(coff.GetSizeOfImage(), kSizeOfImage);
}

TYPED_TEST(PeCoffTest, zero_size_of_image_for_invalid) {
  PeCoff coff(new MemoryFake);
  EXPECT_FALSE(coff.Init());
  EXPECT_FALSE(coff.valid());
  EXPECT_EQ(coff.GetSizeOfImage(), 0);
}

TYPED_TEST(PeCoffTest, step_fails_for_invalid) {
  PeCoff coff(new MemoryFake);
  EXPECT_FALSE(coff.Init());
  EXPECT_FALSE(coff.valid());
  EXPECT_FALSE(coff.Step(0x2000, 0, nullptr, nullptr, nullptr, nullptr));
}

TYPED_TEST(PeCoffTest, step_if_signal_handler_returns_false) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_FALSE(coff.StepIfSignalHandler(0, nullptr, nullptr));
}

TYPED_TEST(PeCoffTest, step_succeeds_when_interface_step_succeeds) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  MockPeCoffInterface* mock_interface = new MockPeCoffInterface;
  EXPECT_CALL(*mock_interface, Step(0x2000, 0, nullptr, nullptr, nullptr, nullptr))
      .WillOnce(::testing::Return(true));
  coff.SetFakePeCoffInterface(mock_interface);
  EXPECT_TRUE(coff.Step(0x2000, 0, nullptr, nullptr, nullptr, nullptr));
}

TYPED_TEST(PeCoffTest, steps_fails_when_interface_step_fails) {
  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  MockPeCoffInterface* mock_interface = new MockPeCoffInterface;
  EXPECT_CALL(*mock_interface, Step(0x2000, 0, nullptr, nullptr, nullptr, nullptr))
      .WillOnce(::testing::Return(false));
  coff.SetFakePeCoffInterface(mock_interface);
  EXPECT_FALSE(coff.Step(0x2000, 0, nullptr, nullptr, nullptr, nullptr));
}

TYPED_TEST(PeCoffTest, returns_correct_memory_ptr) {
  this->GetFake()->Init();
  Memory* memory = this->ReleaseMemory();
  PeCoff coff(memory);
  EXPECT_EQ(memory, coff.memory());
}

TYPED_TEST(PeCoffTest, errors_are_passed_through_from_interface) {
  constexpr uint64_t kErrorAddress = 0x100;
  constexpr ErrorCode kErrorCode = unwindstack::ErrorCode::ERROR_INVALID_COFF;

  this->GetFake()->Init();
  FakePeCoff coff(this->ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_TRUE(coff.valid());

  MockPeCoffInterface* mock_interface = new MockPeCoffInterface;
  coff.SetFakePeCoffInterface(mock_interface);

  ErrorData mock_return_value{kErrorCode, kErrorAddress};
  EXPECT_CALL(*mock_interface, last_error()).WillOnce(::testing::ReturnRef(mock_return_value));
  ErrorData error_data;
  coff.GetLastError(&error_data);
  EXPECT_EQ(kErrorCode, error_data.code);
  EXPECT_EQ(kErrorAddress, error_data.address);

  EXPECT_CALL(*mock_interface, LastErrorCode()).WillOnce(::testing::Return(kErrorCode));
  EXPECT_EQ(kErrorCode, coff.GetLastErrorCode());

  EXPECT_CALL(*mock_interface, LastErrorAddress()).WillOnce(::testing::Return(kErrorAddress));
  EXPECT_EQ(kErrorAddress, coff.GetLastErrorAddress());
}

// Tests that are specific to, or are easier to write specifically for, a single architecture.
using PeCoff32Test = PeCoffTest<PeCoffInterface32>;
using PeCoff64Test = PeCoffTest<PeCoffInterface64>;

TEST_F(PeCoff32Test, returns_correct_arch_for_32bit_pe_coff) {
  GetFake()->Init();
  PeCoff coff(ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_EQ(ARCH_X86, coff.arch());
}

TEST_F(PeCoff64Test, returns_correct_arch_for_64bit_pe_coff) {
  GetFake()->Init();
  PeCoff coff(ReleaseMemory());
  EXPECT_TRUE(coff.Init());
  EXPECT_EQ(ARCH_X86_64, coff.arch());
}

TEST(PeCoffTest, detects_pe_coff_magic_value_for_given_memory) {
  MemoryFake memory;
  constexpr uint16_t kMsDosTwoPointZeroMagicValue = 0x5a4d;
  memory.SetData16(0, kMsDosTwoPointZeroMagicValue);
  EXPECT_TRUE(IsPotentiallyPeCoffFile(&memory));
}

TEST(PeCoffTest, rejects_incorrect_pe_coff_magic_value_for_given_memory) {
  MemoryFake memory;
  constexpr uint16_t kIncorrectMagicValue = 0x5a4e;
  memory.SetData16(0, kIncorrectMagicValue);
  EXPECT_FALSE(IsPotentiallyPeCoffFile(&memory));
}

TEST(PeCoffTest, detects_pe_coff_file_correctly) {
  std::string file = android::base::GetExecutableDirectory() + "/tests/files/libtest.dll";
  EXPECT_TRUE(IsPotentiallyPeCoffFile(file));
}

TEST(PeCoffTest, rejects_non_pe_coff_correctly) {
  std::string file = android::base::GetExecutableDirectory() + "/tests/files/elf32.xz";
  EXPECT_FALSE(IsPotentiallyPeCoffFile(file));
}

}  // namespace unwindstack