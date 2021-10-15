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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unwindstack/DwarfSection.h>
#include <unwindstack/Elf.h>

#include "RegsFake.h"
#include "utils/MemoryFake.h"

namespace unwindstack {

class MockDwarfSection : public DwarfSection {
 public:
  MockDwarfSection(Memory* memory) : DwarfSection(memory) {}
  virtual ~MockDwarfSection() = default;

  MOCK_METHOD(bool, Init, (uint64_t, uint64_t, int64_t), (override));

  MOCK_METHOD(bool, Eval, (const DwarfCie*, Memory*, const DwarfLocations&, Regs*, bool*),
              (override));

  MOCK_METHOD(bool, Log, (uint8_t, uint64_t, const DwarfFde*, ArchEnum arch), (override));

  MOCK_METHOD(void, GetFdes, (std::vector<const DwarfFde*>*), (override));

  MOCK_METHOD(const DwarfFde*, GetFdeFromPc, (uint64_t), (override));

  MOCK_METHOD(bool, GetCfaLocationInfo, (uint64_t, const DwarfFde*, DwarfLocations*, ArchEnum arch),
              (override));

  MOCK_METHOD(uint64_t, GetCieOffsetFromFde32, (uint32_t), (override));

  MOCK_METHOD(uint64_t, GetCieOffsetFromFde64, (uint64_t), (override));

  MOCK_METHOD(uint64_t, AdjustPcFromFde, (uint64_t), (override));
};

class DwarfSectionTest : public ::testing::Test {
 protected:
  void SetUp() override { section_.reset(new MockDwarfSection(&memory_)); }

  MemoryFake memory_;
  std::unique_ptr<MockDwarfSection> section_;
  static RegsFake regs_;
};

RegsFake DwarfSectionTest::regs_(10);

TEST_F(DwarfSectionTest, Step_fail_fde) {
  EXPECT_CALL(*section_, GetFdeFromPc(0x1000)).WillOnce(::testing::Return(nullptr));

  bool finished;
  bool is_signal_frame;
  ASSERT_FALSE(section_->Step(0x1000, nullptr, nullptr, &finished, &is_signal_frame));
}

TEST_F(DwarfSectionTest, Step_fail_cie_null) {
  DwarfFde fde{};
  fde.pc_end = 0x2000;
  fde.cie = nullptr;

  EXPECT_CALL(*section_, GetFdeFromPc(0x1000)).WillOnce(::testing::Return(&fde));

  bool finished;
  bool is_signal_frame;
  ASSERT_FALSE(section_->Step(0x1000, &regs_, nullptr, &finished, &is_signal_frame));
}

TEST_F(DwarfSectionTest, Step_fail_cfa_location) {
  DwarfCie cie{};
  DwarfFde fde{};
  fde.pc_end = 0x2000;
  fde.cie = &cie;

  EXPECT_CALL(*section_, GetFdeFromPc(0x1000)).WillOnce(::testing::Return(&fde));
  EXPECT_CALL(*section_, GetCfaLocationInfo(0x1000, &fde, ::testing::_, ::testing::_))
      .WillOnce(::testing::Return(false));

  bool finished;
  bool is_signal_frame;
  ASSERT_FALSE(section_->Step(0x1000, &regs_, nullptr, &finished, &is_signal_frame));
}

TEST_F(DwarfSectionTest, Step_pass) {
  DwarfCie cie{};
  DwarfFde fde{};
  fde.pc_end = 0x2000;
  fde.cie = &cie;

  EXPECT_CALL(*section_, GetFdeFromPc(0x1000)).WillOnce(::testing::Return(&fde));
  EXPECT_CALL(*section_, GetCfaLocationInfo(0x1000, &fde, ::testing::_, ::testing::_))
      .WillOnce(::testing::Return(true));

  MemoryFake process;
  EXPECT_CALL(*section_, Eval(&cie, &process, ::testing::_, &regs_, ::testing::_))
      .WillOnce(::testing::Return(true));

  bool finished;
  bool is_signal_frame;
  ASSERT_TRUE(section_->Step(0x1000, &regs_, &process, &finished, &is_signal_frame));
}

static bool MockGetCfaLocationInfo(::testing::Unused, const DwarfFde* fde, DwarfLocations* loc_regs,
                                   ArchEnum) {
  loc_regs->pc_start = fde->pc_start;
  loc_regs->pc_end = fde->pc_end;
  return true;
}

TEST_F(DwarfSectionTest, Step_cache) {
  DwarfCie cie{};
  DwarfFde fde{};
  fde.pc_start = 0x500;
  fde.pc_end = 0x2000;
  fde.cie = &cie;

  EXPECT_CALL(*section_, GetFdeFromPc(0x1000)).WillOnce(::testing::Return(&fde));
  EXPECT_CALL(*section_, GetCfaLocationInfo(0x1000, &fde, ::testing::_, ::testing::_))
      .WillOnce(::testing::Invoke(MockGetCfaLocationInfo));

  MemoryFake process;
  EXPECT_CALL(*section_, Eval(&cie, &process, ::testing::_, &regs_, ::testing::_))
      .WillRepeatedly(::testing::Return(true));

  bool finished;
  bool is_signal_frame;
  ASSERT_TRUE(section_->Step(0x1000, &regs_, &process, &finished, &is_signal_frame));
  ASSERT_TRUE(section_->Step(0x1000, &regs_, &process, &finished, &is_signal_frame));
  ASSERT_TRUE(section_->Step(0x1500, &regs_, &process, &finished, &is_signal_frame));
}

TEST_F(DwarfSectionTest, Step_cache_not_in_pc) {
  DwarfCie cie{};
  DwarfFde fde0{};
  fde0.pc_start = 0x1000;
  fde0.pc_end = 0x2000;
  fde0.cie = &cie;
  EXPECT_CALL(*section_, GetFdeFromPc(0x1000)).WillOnce(::testing::Return(&fde0));
  EXPECT_CALL(*section_, GetCfaLocationInfo(0x1000, &fde0, ::testing::_, ::testing::_))
      .WillOnce(::testing::Invoke(MockGetCfaLocationInfo));

  MemoryFake process;
  EXPECT_CALL(*section_, Eval(&cie, &process, ::testing::_, &regs_, ::testing::_))
      .WillRepeatedly(::testing::Return(true));

  bool finished;
  bool is_signal_frame;
  ASSERT_TRUE(section_->Step(0x1000, &regs_, &process, &finished, &is_signal_frame));

  DwarfFde fde1{};
  fde1.pc_start = 0x500;
  fde1.pc_end = 0x800;
  fde1.cie = &cie;
  EXPECT_CALL(*section_, GetFdeFromPc(0x600)).WillOnce(::testing::Return(&fde1));
  EXPECT_CALL(*section_, GetCfaLocationInfo(0x600, &fde1, ::testing::_, ::testing::_))
      .WillOnce(::testing::Invoke(MockGetCfaLocationInfo));

  ASSERT_TRUE(section_->Step(0x600, &regs_, &process, &finished, &is_signal_frame));
  ASSERT_TRUE(section_->Step(0x700, &regs_, &process, &finished, &is_signal_frame));
}

}  // namespace unwindstack
