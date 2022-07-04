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

#include <unwindstack/PeCoffInterface.h>
#include "PeCoffUnwindInfos.h"
#include "unwindstack/Error.h"
#include "utils/MemoryFake.h"

#include <gtest/gtest.h>

namespace unwindstack {
// Unwind infos are indexed using their relative virtual address, which the
// PeCoffUnwindInfos class internally converts to a file offset. We add a
// simple mapping using a section that captures all possible addresses with
// the constants below.
constexpr uint32_t kSectionSize = 0x1000;
constexpr uint32_t kVmAddress = 0x6600;
constexpr uint32_t kFileOffset = 0x4000;
constexpr uint32_t kSectionFlags = 0x40000040;
std::vector<unwindstack::Section> kSections{unwindstack::Section{
    "all_addresses", kSectionSize, kVmAddress, kSectionSize, kFileOffset, kSectionFlags}};

class PeCoffUnwindInfosTest : public ::testing::Test {
 public:
  PeCoffUnwindInfosTest() : memory_fake_(new MemoryFake) {}
  ~PeCoffUnwindInfosTest() override = default;

  uint64_t SetUnwindInfoHeaderAtOffset(uint64_t offset, uint8_t num_codes, bool chained) {
    uint8_t flags = 0x00;
    if (chained) {
      // To get the flags, this value will be shifted right by 3. To get chained info,
      // after shifting, this value must be 0x04.
      flags |= 0x04 << 3;
    }
    // Only version "1" is documented, set it here. But we also support undocumented version "2",
    // which has a dedicated test below.
    uint8_t version_and_flags = flags | 0x01;
    memory_fake_->SetData8(offset, version_and_flags);
    offset += sizeof(uint8_t);

    // prolog_size, actual value doesn't matter for tests here.
    memory_fake_->SetData8(offset, 0x20);
    offset += sizeof(uint8_t);
    memory_fake_->SetData8(offset, num_codes);
    offset += sizeof(uint8_t);

    // frame_register_and_offset, actual value doesn't matter for tests here.
    memory_fake_->SetData8(offset, 0x22);
    offset += sizeof(uint8_t);
    return offset;
  }
  uint64_t SetUnwindOpCodeOrFrameOffsetAtOffset(uint64_t offset, uint16_t value) {
    memory_fake_->SetData16(offset, value);
    return offset + sizeof(uint16_t);
  }

  // Exception handler and chained info are  exclusive, only one of them can be present,
  // if at all.
  uint64_t SetExceptionHandlerOffsetAtOffset(uint64_t offset, uint64_t exception_handler_offset) {
    memory_fake_->SetData64(offset, exception_handler_offset);
    return offset + sizeof(uint64_t);
  }

  uint64_t SetChainedInfoOffsetAtOffset(uint64_t offset) {
    // The PeCoffUnwindInfo class does not interpret chained infos, so it doesn't really
    // matter what values we put here.
    RuntimeFunction chained_function{0x100, 0x200, kFileOffset};
    memory_fake_->SetData32(offset, chained_function.start_address);
    offset += sizeof(uint32_t);
    memory_fake_->SetData32(offset, chained_function.end_address);
    offset += sizeof(uint32_t);
    memory_fake_->SetData32(offset, chained_function.unwind_info_offset);
    offset += sizeof(uint32_t);
    return offset;
  }

  MemoryFake* GetMemoryFake() { return memory_fake_.get(); }

 private:
  std::unique_ptr<MemoryFake> memory_fake_;
};

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_succeeds_on_well_formed_data_no_chained_info) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 2, false);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_TRUE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_NONE, unwind_infos->GetLastError().code);

  EXPECT_EQ(2, unwind_info->num_codes);
}

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_succeeds_multiple_times) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 2, false);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);

  offset = SetUnwindInfoHeaderAtOffset(offset, 4, false);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x5678);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x8765);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x8756);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x7658);

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_TRUE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));

  // This should read from the cache, though we don't verify that here. The returned
  // data should be the same, though.
  UnwindInfo* unwind_info2;
  EXPECT_TRUE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info2));

  EXPECT_EQ(unwind_info->version_and_flags, unwind_info2->version_and_flags);
  EXPECT_EQ(unwind_info->prolog_size, unwind_info2->prolog_size);
  EXPECT_EQ(unwind_info->num_codes, unwind_info2->num_codes);
  EXPECT_EQ(unwind_info->frame_register_and_offset, unwind_info2->frame_register_and_offset);
}

TEST_F(PeCoffUnwindInfosTest,
       get_unwind_info_succeeds_on_well_formed_data_chained_info_even_number_of_opcodes) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 2, true);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);

  offset = SetChainedInfoOffsetAtOffset(offset);

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_TRUE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_NONE, unwind_infos->GetLastError().code);

  EXPECT_EQ(2, unwind_info->num_codes);
}

TEST_F(PeCoffUnwindInfosTest,
       get_unwind_info_succeeds_on_well_formed_data_chained_info_odd_number_of_opcodes) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 3, true);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x3124);
  // Add padding, per specification.
  GetMemoryFake()->SetData16(offset, 0x0000);
  offset += sizeof(uint16_t);

  offset = SetChainedInfoOffsetAtOffset(offset);

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_TRUE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_NONE, unwind_infos->GetLastError().code);

  EXPECT_EQ(3, unwind_info->num_codes);
}

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_succeeds_with_exception_handler_data) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 2, false);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);
  offset = SetExceptionHandlerOffsetAtOffset(offset, 0x8000);

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_TRUE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_NONE, unwind_infos->GetLastError().code);

  EXPECT_EQ(2, unwind_info->num_codes);
}

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_succeeds_on_version_2) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 2, false);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);

  // Set the version to 2. Note that this also clears the flags.
  GetMemoryFake()->SetData8(kFileOffset, 0x02);

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_TRUE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_NONE, unwind_infos->GetLastError().code);

  EXPECT_EQ(2, unwind_info->num_codes);
}

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_fails_on_bad_version) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 2, false);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);

  for (uint8_t bad_version : {0x00, 0x03, 0x04, 0x05, 0x06, 0x07}) {
    // Clobber the version.
    GetMemoryFake()->SetData8(kFileOffset, bad_version);

    std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
        CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

    UnwindInfo* unwind_info;
    EXPECT_FALSE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
    EXPECT_EQ(ERROR_INVALID_COFF, unwind_infos->GetLastError().code);
  }
}

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_fails_on_bad_memory) {
  GetMemoryFake()->SetData8(kFileOffset, 0x1);
  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_FALSE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_MEMORY_INVALID, unwind_infos->GetLastError().code);

  // We read the first 4 bytes with a GetFully, so we must fail on this address
  // (and not 0x6001, which is the first missing address).
  EXPECT_EQ(kFileOffset, unwind_infos->GetLastError().address);
}

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_fails_on_incomplete_op_codes_memory) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 3, false);

  // Since we are using GetFully to get all op codes in a single memory read, we
  // expect the error on this offset.
  const uint64_t expected_error_address = offset;

  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_FALSE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_MEMORY_INVALID, unwind_infos->GetLastError().code);
  EXPECT_EQ(expected_error_address, unwind_infos->GetLastError().address);
}

TEST_F(PeCoffUnwindInfosTest, get_unwind_info_fails_on_incomplete_chained_info) {
  uint64_t offset = kFileOffset;
  offset = SetUnwindInfoHeaderAtOffset(offset, 2, true);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x1234);
  offset = SetUnwindOpCodeOrFrameOffsetAtOffset(offset, 0x2134);

  // We expect chained info but don't set it here, so getting the unwind info below
  // must fail on this address.
  const uint64_t expected_error_address = offset;

  std::unique_ptr<PeCoffUnwindInfos> unwind_infos(
      CreatePeCoffUnwindInfos(GetMemoryFake(), kSections));

  UnwindInfo* unwind_info;
  EXPECT_FALSE(unwind_infos->GetUnwindInfo(kVmAddress, &unwind_info));
  EXPECT_EQ(ERROR_MEMORY_INVALID, unwind_infos->GetLastError().code);
  EXPECT_EQ(expected_error_address, unwind_infos->GetLastError().address);
}

}  // namespace unwindstack