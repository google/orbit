/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <stdint.h>

#include <gtest/gtest.h>

#include <unwindstack/DwarfError.h>

#include "DwarfEhFrame.h"
#include "DwarfEncoding.h"

#include "LogFake.h"
#include "utils/MemoryFake.h"

namespace unwindstack {

template <typename TypeParam>
class DwarfEhFrameTest : public ::testing::Test {
 protected:
  void SetUp() override {
    memory_.Clear();
    eh_frame_ = new DwarfEhFrame<TypeParam>(&memory_);
    ResetLogs();
  }

  void TearDown() override { delete eh_frame_; }

  MemoryFake memory_;
  DwarfEhFrame<TypeParam>* eh_frame_ = nullptr;
};
TYPED_TEST_SUITE_P(DwarfEhFrameTest);

// NOTE: All test class variables need to be referenced as this->.

// Only verify different cie/fde format. All other DwarfSection corner
// cases are tested in DwarfDebugFrameTest.cpp.

TYPED_TEST_P(DwarfEhFrameTest, GetFdeCieFromOffset32) {
  // CIE 32 information.
  this->memory_.SetData32(0x5000, 0xfc);
  // Indicates this is a cie for eh_frame.
  this->memory_.SetData32(0x5004, 0);
  constexpr uint64_t kCieDataOffset = 0x5008;

  // We force size of target address pointers to 4 bytes using augmentation data to avoid
  // failures due to incorrect .eh_frame section parsing (which uses a fixed "4" as the
  // value size), which is not easily fixed due to side effects for other cases. In practice,
  // .eh_frame encoding typically (always?) is DW_EH_PE_pcrel | DW_EH_PE_sdata4 (0x1b), which
  // thus uses a size of 4 bytes.
  std::vector<uint8_t> data{
      1,               // version
      'z', 'R', '\0',  // augmentation string
      16,              // code alignment factor
      32,              // data alignment factor
      2,               // return address register
      1,               // augmentation data length, ULEB128
      0x03             // augmentation data (DW_EH_PE_udata4)
  };
  this->memory_.SetMemory(kCieDataOffset, data);

  // FDE 32 information.
  this->memory_.SetData32(0x5100, 0xfc);
  this->memory_.SetData32(0x5104, 0x104);
  this->memory_.SetData32(0x5108, 0x1500);
  this->memory_.SetData32(0x510c, 0x200);

  // Augmentation size, ULEB128 encoding, must be present as 'z' is presen in the
  // augmentation string.
  constexpr uint64_t kFdeAugmentationSizeOffset = 0x5110;
  this->memory_.SetData8(0x5110, 0x0);

  const DwarfFde* fde = this->eh_frame_->GetFdeFromOffset(0x5100);
  ASSERT_TRUE(fde != nullptr);
  EXPECT_EQ(0x5000U, fde->cie_offset);

  // This is the offset of the augmentation size in the FDE (kFdeAugmentationSizeOffset) + 1, for
  // the one byte that represents the size of the augmentation data.
  EXPECT_EQ(kFdeAugmentationSizeOffset + 1, fde->cfa_instructions_offset);
  EXPECT_EQ(0x5200U, fde->cfa_instructions_end);
  EXPECT_EQ(0x6608U, fde->pc_start);
  EXPECT_EQ(0x6808U, fde->pc_end);
  EXPECT_EQ(0U, fde->lsda_address);

  const DwarfCie* cie = fde->cie;
  ASSERT_TRUE(cie != nullptr);
  EXPECT_EQ(1U, cie->version);
  EXPECT_EQ(DW_EH_PE_udata4, cie->fde_address_encoding);
  EXPECT_EQ(DW_EH_PE_omit, cie->lsda_encoding);
  EXPECT_EQ(0U, cie->segment_size);
  EXPECT_EQ('z', cie->augmentation_string[0]);
  EXPECT_EQ('R', cie->augmentation_string[1]);
  EXPECT_EQ('\0', cie->augmentation_string[2]);
  EXPECT_EQ(0U, cie->personality_handler);
  EXPECT_EQ(kCieDataOffset + data.size(), cie->cfa_instructions_offset);
  EXPECT_EQ(0x5100U, cie->cfa_instructions_end);
  EXPECT_EQ(16U, cie->code_alignment_factor);
  EXPECT_EQ(32U, cie->data_alignment_factor);
  EXPECT_EQ(2U, cie->return_address_register);
}

TYPED_TEST_P(DwarfEhFrameTest, GetFdeCieFromOffset64) {
  // CIE 64 information.
  this->memory_.SetData32(0x5000, 0xffffffff);
  this->memory_.SetData64(0x5004, 0xfc);
  // Indicates this is a cie for eh_frame.
  this->memory_.SetData64(0x500c, 0);

  // We force size of target address pointers to 4 bytes using augmentation data to avoid
  // failures due to incorrect .eh_frame section parsing (which uses a fixed "4" as the
  // value size), which is not easily fixed due to side effects for other cases. In practice,
  // .eh_frame encoding typically (always?) is DW_EH_PE_pcrel | DW_EH_PE_sdata4 (0x1b), which
  // thus uses a size of 4 bytes.
  constexpr uint64_t kCieDataOffset = 0x5014;
  std::vector<uint8_t> data{
      1,               // version
      'z', 'R', '\0',  // augmentation string
      16,              // code alignment factor
      32,              // data alignment factor
      2,               // return address register
      1,               // augmentation data length, ULEB128
      0x03             // augmentation data (DW_EH_PE_udata4)
  };
  this->memory_.SetMemory(kCieDataOffset, data);

  // FDE 64 information.
  this->memory_.SetData32(0x5100, 0xffffffff);
  this->memory_.SetData64(0x5104, 0xfc);
  this->memory_.SetData64(0x510c, 0x10c);
  constexpr uint64_t kPcStartOffsetInFile = 0x5114;
  constexpr uint32_t kPcStartValueInFile = 0x1500;
  this->memory_.SetData32(kPcStartOffsetInFile, kPcStartValueInFile);
  this->memory_.SetData32(0x5118, 0x200);

  // Augmentation size, ULEB128 encoding, must be present as 'z' is presen in the
  // augmentation string.
  constexpr uint64_t kFdeAugmentationSizeOffset = 0x511c;
  this->memory_.SetData8(kFdeAugmentationSizeOffset, 0x0);

  const DwarfFde* fde = this->eh_frame_->GetFdeFromOffset(0x5100);
  ASSERT_TRUE(fde != nullptr);
  EXPECT_EQ(0x5000U, fde->cie_offset);
  EXPECT_EQ(kFdeAugmentationSizeOffset + 1, fde->cfa_instructions_offset);
  EXPECT_EQ(0x5208U, fde->cfa_instructions_end);
  EXPECT_EQ(kPcStartOffsetInFile + kPcStartValueInFile, fde->pc_start);
  EXPECT_EQ(kPcStartOffsetInFile + kPcStartValueInFile + 0x200, fde->pc_end);
  EXPECT_EQ(0U, fde->lsda_address);

  const DwarfCie* cie = fde->cie;
  ASSERT_TRUE(cie != nullptr);
  EXPECT_EQ(1U, cie->version);
  EXPECT_EQ(DW_EH_PE_udata4, cie->fde_address_encoding);
  EXPECT_EQ(DW_EH_PE_omit, cie->lsda_encoding);
  EXPECT_EQ(0U, cie->segment_size);
  EXPECT_EQ('z', cie->augmentation_string[0]);
  EXPECT_EQ('R', cie->augmentation_string[1]);
  EXPECT_EQ('\0', cie->augmentation_string[2]);
  EXPECT_EQ(0U, cie->personality_handler);
  EXPECT_EQ(kCieDataOffset + data.size(), cie->cfa_instructions_offset);
  EXPECT_EQ(0x5108U, cie->cfa_instructions_end);
  EXPECT_EQ(16U, cie->code_alignment_factor);
  EXPECT_EQ(32U, cie->data_alignment_factor);
  EXPECT_EQ(2U, cie->return_address_register);
}

REGISTER_TYPED_TEST_SUITE_P(DwarfEhFrameTest, GetFdeCieFromOffset32, GetFdeCieFromOffset64);

typedef ::testing::Types<uint32_t, uint64_t> DwarfEhFrameTestTypes;
INSTANTIATE_TYPED_TEST_SUITE_P(Libunwindstack, DwarfEhFrameTest, DwarfEhFrameTestTypes);

}  // namespace unwindstack
