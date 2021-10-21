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
#include <unwindstack/DwarfSection.h>
#include <unwindstack/Elf.h>

#include "DwarfEncoding.h"

#include "LogFake.h"
#include "utils/DwarfSectionImplFake.h"
#include "utils/MemoryFake.h"
#include "utils/RegsFake.h"

namespace unwindstack {

template <typename TypeParam>
class DwarfSectionImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    memory_.Clear();
    section_ = new DwarfSectionImplFake<TypeParam>(&memory_);
    ResetLogs();
  }

  void TearDown() override { delete section_; }

  MemoryFake memory_;
  DwarfSectionImplFake<TypeParam>* section_ = nullptr;
};
TYPED_TEST_SUITE_P(DwarfSectionImplTest);

// NOTE: All test class variables need to be referenced as this->.

TYPED_TEST_P(DwarfSectionImplTest, GetCieFromOffset_fail_should_not_cache) {
  ASSERT_TRUE(this->section_->GetCieFromOffset(0x4000) == nullptr);
  EXPECT_EQ(DWARF_ERROR_MEMORY_INVALID, this->section_->LastErrorCode());
  EXPECT_EQ(0x4000U, this->section_->LastErrorAddress());

  this->section_->FakeClearError();
  ASSERT_TRUE(this->section_->GetCieFromOffset(0x4000) == nullptr);
  EXPECT_EQ(DWARF_ERROR_MEMORY_INVALID, this->section_->LastErrorCode());
  EXPECT_EQ(0x4000U, this->section_->LastErrorAddress());
}

TYPED_TEST_P(DwarfSectionImplTest, GetFdeFromOffset_fail_should_not_cache) {
  ASSERT_TRUE(this->section_->GetFdeFromOffset(0x4000) == nullptr);
  EXPECT_EQ(DWARF_ERROR_MEMORY_INVALID, this->section_->LastErrorCode());
  EXPECT_EQ(0x4000U, this->section_->LastErrorAddress());

  this->section_->FakeClearError();
  ASSERT_TRUE(this->section_->GetFdeFromOffset(0x4000) == nullptr);
  EXPECT_EQ(DWARF_ERROR_MEMORY_INVALID, this->section_->LastErrorCode());
  EXPECT_EQ(0x4000U, this->section_->LastErrorAddress());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_expr_eval_fail) {
  DwarfCie cie{.version = 3, .return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[9] = 0x3000;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x2, 0x5002}};
  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_MEMORY_INVALID, this->section_->LastErrorCode());
  EXPECT_EQ(0x5000U, this->section_->LastErrorAddress());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_expr_no_stack) {
  DwarfCie cie{.version = 3, .return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[9] = 0x3000;
  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x96, 0x96, 0x96});
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x2, 0x5002}};
  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_STATE, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_expr) {
  DwarfCie cie{.version = 3, .return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[9] = 0x3000;
  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x80});
  TypeParam cfa_value = 0x12345;
  this->memory_.SetMemory(0x80000000, &cfa_value, sizeof(cfa_value));
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_EXPRESSION, {0x4, 0x5004}};
  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_val_expr) {
  DwarfCie cie{.version = 3, .return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[9] = 0x3000;
  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x80});
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x4, 0x5004}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  ASSERT_FALSE(finished);
  EXPECT_EQ(0x80000000U, regs.sp());
  EXPECT_EQ(0x20U, regs.pc());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_expr_is_register) {
  DwarfCie cie{.version = 3, .return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[9] = 0x3000;
  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x50, 0x96, 0x96});
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x2, 0x5002}};
  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_NOT_IMPLEMENTED, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_bad_regs) {
  DwarfCie cie{.return_address_register = 60};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_no_cfa) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_CFA_NOT_DEFINED, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_bad) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {20, 0}};
  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());

  this->section_->FakeClearError();
  loc_regs.erase(CFA_REG);
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_INVALID, {0, 0}};
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());

  this->section_->FakeClearError();
  loc_regs.erase(CFA_REG);
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_OFFSET, {0, 0}};
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());

  this->section_->FakeClearError();
  loc_regs.erase(CFA_REG);
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_VAL_OFFSET, {0, 0}};
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_register_prev) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[9] = 0x3000;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {9, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_FALSE(finished);
  EXPECT_EQ(0x20U, regs.pc());
  EXPECT_EQ(0x3000U, regs.sp());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_cfa_register_from_value) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[6] = 0x4000;
  regs[9] = 0x3000;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {6, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_FALSE(finished);
  EXPECT_EQ(0x20U, regs.pc());
  EXPECT_EQ(0x4000U, regs.sp());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_double_indirection) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[1] = 0x100;
  regs[3] = 0x300;
  regs[8] = 0x10;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[1] = DwarfLocation{DWARF_LOCATION_REGISTER, {3, 1}};
  loc_regs[9] = DwarfLocation{DWARF_LOCATION_REGISTER, {1, 2}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(0x301U, regs[1]);
  EXPECT_EQ(0x300U, regs[3]);
  EXPECT_EQ(0x10U, regs[8]);
  EXPECT_EQ(0x102U, regs[9]);
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_register_reference_chain) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[0] = 0x10;
  regs[1] = 0x20;
  regs[2] = 0x30;
  regs[3] = 0x40;
  regs[4] = 0x50;
  regs[5] = 0x60;
  regs[8] = 0x20;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[1] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 1}};
  loc_regs[2] = DwarfLocation{DWARF_LOCATION_REGISTER, {1, 2}};
  loc_regs[3] = DwarfLocation{DWARF_LOCATION_REGISTER, {2, 3}};
  loc_regs[4] = DwarfLocation{DWARF_LOCATION_REGISTER, {3, 4}};
  loc_regs[5] = DwarfLocation{DWARF_LOCATION_REGISTER, {4, 5}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(0x10U, regs[0]);
  EXPECT_EQ(0x11U, regs[1]);
  EXPECT_EQ(0x22U, regs[2]);
  EXPECT_EQ(0x33U, regs[3]);
  EXPECT_EQ(0x44U, regs[4]);
  EXPECT_EQ(0x55U, regs[5]);
  EXPECT_EQ(0x20U, regs[8]);
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_dex_pc) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[0] = 0x10;
  regs[8] = 0x20;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[1] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x8, 0x5008}};
  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 'D', 'E', 'X', '1', 0x13, 0x08, 0x11});
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(0x10U, regs[0]);
  EXPECT_EQ(0x20U, regs[8]);
  EXPECT_EQ(0x11U, regs.dex_pc());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_invalid_register) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[8] = 0x10;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[1] = DwarfLocation{DWARF_LOCATION_REGISTER, {10, 0}};
  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_different_reg_locations) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  if (sizeof(TypeParam) == sizeof(uint64_t)) {
    this->memory_.SetData64(0x2150, 0x12345678abcdef00ULL);
  } else {
    this->memory_.SetData32(0x2150, 0x12345678);
  }

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[3] = 0x234;
  regs[5] = 0x10;
  regs[8] = 0x2100;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[1] = DwarfLocation{DWARF_LOCATION_VAL_OFFSET, {0x100, 0}};
  loc_regs[2] = DwarfLocation{DWARF_LOCATION_OFFSET, {0x50, 0}};
  loc_regs[3] = DwarfLocation{DWARF_LOCATION_UNDEFINED, {0, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_FALSE(finished);
  EXPECT_EQ(0x10U, regs.pc());
  EXPECT_EQ(0x2100U, regs.sp());
  EXPECT_EQ(0x2200U, regs[1]);
  EXPECT_EQ(0x234U, regs[3]);
  if (sizeof(TypeParam) == sizeof(uint64_t)) {
    EXPECT_EQ(0x12345678abcdef00ULL, regs[2]);
  } else {
    EXPECT_EQ(0x12345678U, regs[2]);
  }
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_return_address_undefined) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[8] = 0x10;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[5] = DwarfLocation{DWARF_LOCATION_UNDEFINED, {0, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_TRUE(finished);
  EXPECT_EQ(0U, regs.pc());
  EXPECT_EQ(0x10U, regs.sp());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_pc_zero) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0;
  regs[8] = 0x10;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_TRUE(finished);
  EXPECT_EQ(0U, regs.pc());
  EXPECT_EQ(0x10U, regs.sp());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_return_address) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[8] = 0x10;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_FALSE(finished);
  EXPECT_EQ(0x20U, regs.pc());
  EXPECT_EQ(0x10U, regs.sp());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_ignore_large_reg_loc) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[5] = 0x20;
  regs[8] = 0x10;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  // This should not result in any errors.
  loc_regs[20] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_FALSE(finished);
  EXPECT_EQ(0x20U, regs.pc());
  EXPECT_EQ(0x10U, regs.sp());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_reg_expr) {
  DwarfCie cie{.version = 3, .return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[8] = 0x3000;
  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x80});
  TypeParam cfa_value = 0x12345;
  this->memory_.SetMemory(0x80000000, &cfa_value, sizeof(cfa_value));
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[5] = DwarfLocation{DWARF_LOCATION_EXPRESSION, {0x4, 0x5004}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_FALSE(finished);
  EXPECT_EQ(0x3000U, regs.sp());
  EXPECT_EQ(0x12345U, regs.pc());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_reg_val_expr) {
  DwarfCie cie{.version = 3, .return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  DwarfLocations loc_regs;

  regs.set_pc(0x100);
  regs.set_sp(0x2000);
  regs[8] = 0x3000;
  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x80});
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[5] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x4, 0x5004}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_FALSE(finished);
  EXPECT_EQ(0x3000U, regs.sp());
  EXPECT_EQ(0x80000000U, regs.pc());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_pseudo_register_invalid) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  regs.set_pseudo_reg(11);
  DwarfLocations loc_regs;

  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[1] = DwarfLocation{DWARF_LOCATION_PSEUDO_REGISTER, {20, 0}};
  bool finished;
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());

  loc_regs.clear();
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[12] = DwarfLocation{DWARF_LOCATION_PSEUDO_REGISTER, {20, 0}};
  ASSERT_FALSE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  EXPECT_EQ(DWARF_ERROR_ILLEGAL_VALUE, this->section_->LastErrorCode());
}

TYPED_TEST_P(DwarfSectionImplTest, Eval_pseudo_register) {
  DwarfCie cie{.return_address_register = 5};
  RegsImplFake<TypeParam> regs(10);
  regs.set_pseudo_reg(11);
  DwarfLocations loc_regs;

  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {8, 0}};
  loc_regs[11] = DwarfLocation{DWARF_LOCATION_PSEUDO_REGISTER, {20, 0}};
  bool finished;
  ASSERT_TRUE(this->section_->Eval(&cie, &this->memory_, loc_regs, &regs, &finished));
  uint64_t pseudo_value = 0;
  ASSERT_TRUE(regs.GetPseudoRegister(11, &pseudo_value));
  EXPECT_EQ(20U, pseudo_value);
}

TYPED_TEST_P(DwarfSectionImplTest, GetCfaLocationInfo_cie_not_cached) {
  DwarfCie cie{};
  cie.cfa_instructions_offset = 0x3000;
  cie.cfa_instructions_end = 0x3002;
  DwarfFde fde{};
  fde.cie = &cie;
  fde.cie_offset = 0x8000;
  fde.cfa_instructions_offset = 0x6000;
  fde.cfa_instructions_end = 0x6002;

  this->memory_.SetMemory(0x3000, std::vector<uint8_t>{0x09, 0x02, 0x01});
  this->memory_.SetMemory(0x6000, std::vector<uint8_t>{0x09, 0x04, 0x03});

  DwarfLocations loc_regs;
  ASSERT_TRUE(this->section_->GetCfaLocationInfo(0x100, &fde, &loc_regs, ARCH_UNKNOWN));
  ASSERT_EQ(2U, loc_regs.size());

  auto entry = loc_regs.find(2);
  ASSERT_NE(entry, loc_regs.end());
  ASSERT_EQ(DWARF_LOCATION_REGISTER, entry->second.type);
  ASSERT_EQ(1U, entry->second.values[0]);

  entry = loc_regs.find(4);
  ASSERT_NE(entry, loc_regs.end());
  ASSERT_EQ(DWARF_LOCATION_REGISTER, entry->second.type);
  ASSERT_EQ(3U, entry->second.values[0]);
}

TYPED_TEST_P(DwarfSectionImplTest, GetCfaLocationInfo_cie_cached) {
  DwarfCie cie{};
  cie.cfa_instructions_offset = 0x3000;
  cie.cfa_instructions_end = 0x3002;
  DwarfFde fde{};
  fde.cie = &cie;
  fde.cie_offset = 0x8000;
  fde.cfa_instructions_offset = 0x6000;
  fde.cfa_instructions_end = 0x6002;

  DwarfLocations cie_loc_regs;
  cie_loc_regs[6] = DwarfLocation{DWARF_LOCATION_REGISTER, {4, 0}};
  this->section_->FakeSetCachedCieLocRegs(0x8000, cie_loc_regs);
  this->memory_.SetMemory(0x6000, std::vector<uint8_t>{0x09, 0x04, 0x03});

  DwarfLocations loc_regs;
  ASSERT_TRUE(this->section_->GetCfaLocationInfo(0x100, &fde, &loc_regs, ARCH_UNKNOWN));
  ASSERT_EQ(2U, loc_regs.size());

  auto entry = loc_regs.find(6);
  ASSERT_NE(entry, loc_regs.end());
  ASSERT_EQ(DWARF_LOCATION_REGISTER, entry->second.type);
  ASSERT_EQ(4U, entry->second.values[0]);

  entry = loc_regs.find(4);
  ASSERT_NE(entry, loc_regs.end());
  ASSERT_EQ(DWARF_LOCATION_REGISTER, entry->second.type);
  ASSERT_EQ(3U, entry->second.values[0]);
}

TYPED_TEST_P(DwarfSectionImplTest, Log) {
  DwarfCie cie{};
  cie.cfa_instructions_offset = 0x5000;
  cie.cfa_instructions_end = 0x5001;
  DwarfFde fde{};
  fde.cie = &cie;
  fde.cfa_instructions_offset = 0x6000;
  fde.cfa_instructions_end = 0x6001;

  this->memory_.SetMemory(0x5000, std::vector<uint8_t>{0x00});
  this->memory_.SetMemory(0x6000, std::vector<uint8_t>{0xc2});
  ASSERT_TRUE(this->section_->Log(2, 0x1000, &fde, ARCH_UNKNOWN));

  ASSERT_EQ(
      "4 unwind     DW_CFA_nop\n"
      "4 unwind     Raw Data: 0x00\n"
      "4 unwind     DW_CFA_restore register(2)\n"
      "4 unwind     Raw Data: 0xc2\n",
      GetFakeLogPrint());
  ASSERT_EQ("", GetFakeLogBuf());
}

REGISTER_TYPED_TEST_SUITE_P(DwarfSectionImplTest, GetCieFromOffset_fail_should_not_cache,
                            GetFdeFromOffset_fail_should_not_cache, Eval_cfa_expr_eval_fail,
                            Eval_cfa_expr_no_stack, Eval_cfa_expr_is_register, Eval_cfa_expr,
                            Eval_cfa_val_expr, Eval_bad_regs, Eval_no_cfa, Eval_cfa_bad,
                            Eval_cfa_register_prev, Eval_cfa_register_from_value,
                            Eval_double_indirection, Eval_register_reference_chain, Eval_dex_pc,
                            Eval_invalid_register, Eval_different_reg_locations,
                            Eval_return_address_undefined, Eval_pc_zero, Eval_return_address,
                            Eval_ignore_large_reg_loc, Eval_reg_expr, Eval_reg_val_expr,
                            Eval_pseudo_register_invalid, Eval_pseudo_register,
                            GetCfaLocationInfo_cie_not_cached, GetCfaLocationInfo_cie_cached, Log);

typedef ::testing::Types<uint32_t, uint64_t> DwarfSectionImplTestTypes;
INSTANTIATE_TYPED_TEST_SUITE_P(Libunwindstack, DwarfSectionImplTest, DwarfSectionImplTestTypes);

}  // namespace unwindstack
