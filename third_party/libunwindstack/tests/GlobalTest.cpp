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

#include <stdint.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include <unwindstack/Elf.h>
#include <unwindstack/Global.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>

#include "ElfFake.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace unwindstack {

class GlobalMock : public Global {
 public:
  explicit GlobalMock(std::shared_ptr<Memory>& memory) : Global(memory) {}
  GlobalMock(std::shared_ptr<Memory>& memory, std::vector<std::string>& search_libs)
      : Global(memory, search_libs) {}

  MOCK_METHOD(bool, ReadVariableData, (uint64_t), (override));

  MOCK_METHOD(void, ProcessArch, (), (override));

  void TestFindAndReadVariable(Maps* maps, const char* var_str) {
    FindAndReadVariable(maps, var_str);
  }
};

class GlobalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    maps_.reset(
        new BufferMaps("10000-11000 r--p 0000 00:00 0 first.so\n"
                       "11000-12000 r-xp 1000 00:00 0 first.so\n"
                       "12000-13000 rw-p 2000 00:00 0 first.so\n"

                       "20000-22000 r--p 0000 00:00 0 second.so\n"
                       "22000-23000 rw-p 2000 00:00 0 second.so\n"

                       "30000-31000 r--p 0000 00:00 0 third.so\n"
                       "31000-32000 ---p 0000 00:00 0\n"
                       "32000-33000 r-xp 1000 00:00 0 third.so\n"
                       "33000-34000 rw-p 2000 00:00 0 third.so\n"

                       "40000-42000 r--p 0000 00:00 0 fourth.so\n"
                       "42000-43000 rw-p 0000 00:00 0 fourth.so\n"));
    ASSERT_TRUE(maps_->Parse());
    ASSERT_EQ(11U, maps_->Total());

    elf_fakes_.push_back(new ElfInterfaceFake(nullptr));
    elf_fakes_.push_back(new ElfInterfaceFake(nullptr));
    elf_fakes_.push_back(new ElfInterfaceFake(nullptr));
    elf_fakes_.push_back(new ElfInterfaceFake(nullptr));

    ElfFake* elf_fake = new ElfFake(nullptr);
    elf_fake->FakeSetValid(true);
    elf_fake->FakeSetInterface(elf_fakes_[0]);
    elf_fakes_[0]->FakeSetDataVaddrStart(0x2000);
    elf_fakes_[0]->FakeSetDataVaddrEnd(0x3000);
    elf_fakes_[0]->FakeSetDataOffset(0x2000);
    auto map_info = maps_->Find(0x10000);
    map_info->GetElfFields().elf_.reset(elf_fake);

    elf_fake = new ElfFake(nullptr);
    elf_fake->FakeSetValid(true);
    elf_fake->FakeSetInterface(elf_fakes_[1]);
    elf_fakes_[1]->FakeSetDataVaddrStart(0x2000);
    elf_fakes_[1]->FakeSetDataVaddrEnd(0x3000);
    elf_fakes_[1]->FakeSetDataOffset(0x2000);
    map_info = maps_->Find(0x20000);
    map_info->GetElfFields().elf_.reset(elf_fake);

    elf_fake = new ElfFake(nullptr);
    elf_fake->FakeSetValid(true);
    elf_fake->FakeSetInterface(elf_fakes_[2]);
    elf_fakes_[2]->FakeSetDataVaddrStart(0x2000);
    elf_fakes_[2]->FakeSetDataVaddrEnd(0x3000);
    elf_fakes_[2]->FakeSetDataOffset(0x2000);
    map_info = maps_->Find(0x30000);
    map_info->GetElfFields().elf_.reset(elf_fake);

    elf_fake = new ElfFake(nullptr);
    elf_fake->FakeSetValid(true);
    elf_fake->FakeSetInterface(elf_fakes_[3]);
    elf_fakes_[3]->FakeSetDataVaddrStart(00);
    elf_fakes_[3]->FakeSetDataVaddrEnd(0x1000);
    elf_fakes_[3]->FakeSetDataOffset(0);
    map_info = maps_->Find(0x40000);
    map_info->GetElfFields().elf_.reset(elf_fake);

    global_.reset(new GlobalMock(empty_));
  }

  std::shared_ptr<Memory> empty_;
  std::unique_ptr<BufferMaps> maps_;
  std::unique_ptr<GlobalMock> global_;
  std::vector<ElfInterfaceFake*> elf_fakes_;
};

TEST_F(GlobalTest, ro_rx_rw) {
  std::string global_var("fake_global");
  elf_fakes_[0]->FakeSetGlobalVariable(global_var, 0x2010);
  EXPECT_CALL(*global_, ReadVariableData(0x12010)).WillOnce(Return(true));

  global_->TestFindAndReadVariable(maps_.get(), global_var.c_str());
}

TEST_F(GlobalTest, ro_rx_rw_searchable) {
  std::vector<std::string> search_libs = {"first.so"};
  global_.reset(new GlobalMock(empty_, search_libs));

  std::string global_var("fake_global");
  elf_fakes_[0]->FakeSetGlobalVariable(global_var, 0x2010);
  EXPECT_CALL(*global_, ReadVariableData(0x12010)).WillOnce(Return(true));

  global_->TestFindAndReadVariable(maps_.get(), global_var.c_str());
}

TEST_F(GlobalTest, ro_rx_rw_not_searchable) {
  std::vector<std::string> search_libs = {"second.so"};
  global_.reset(new GlobalMock(empty_, search_libs));

  std::string global_var("fake_global");
  elf_fakes_[0]->FakeSetGlobalVariable(global_var, 0x2010);

  global_->TestFindAndReadVariable(maps_.get(), global_var.c_str());
}

TEST_F(GlobalTest, ro_rw) {
  std::string global_var("fake_global");
  elf_fakes_[1]->FakeSetGlobalVariable(global_var, 0x2010);
  EXPECT_CALL(*global_, ReadVariableData(0x22010)).WillOnce(Return(true));

  global_->TestFindAndReadVariable(maps_.get(), global_var.c_str());
}

TEST_F(GlobalTest, ro_blank_rx_rw) {
  std::string global_var("fake_global");
  elf_fakes_[2]->FakeSetGlobalVariable(global_var, 0x2010);
  EXPECT_CALL(*global_, ReadVariableData(0x33010)).WillOnce(Return(true));

  global_->TestFindAndReadVariable(maps_.get(), global_var.c_str());
}

TEST_F(GlobalTest, ro_rw_with_zero_offset) {
  std::string global_var("fake_global");
  elf_fakes_[3]->FakeSetGlobalVariable(global_var, 0x10);
  EXPECT_CALL(*global_, ReadVariableData(0x42010)).WillOnce(Return(true));

  global_->TestFindAndReadVariable(maps_.get(), global_var.c_str());
}

}  // namespace unwindstack
