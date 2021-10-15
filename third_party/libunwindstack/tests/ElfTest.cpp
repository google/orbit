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

#include <elf.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unwindstack/Elf.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/RegsArm.h>
#include <unwindstack/SharedString.h>

#include "ElfFake.h"
#include "ElfTestUtils.h"
#include "LogFake.h"
#include "utils/MemoryFake.h"

#if !defined(PT_ARM_EXIDX)
#define PT_ARM_EXIDX 0x70000001
#endif

namespace unwindstack {

class ElfTest : public ::testing::Test {
 protected:
  void SetUp() override {
    memory_ = new MemoryFake;
  }

  void InitElf32(uint32_t machine_type) {
    Elf32_Ehdr ehdr;
    TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, machine_type);

    ehdr.e_phoff = 0x100;
    ehdr.e_ehsize = sizeof(ehdr);
    ehdr.e_phentsize = sizeof(Elf32_Phdr);
    ehdr.e_phnum = 1;
    ehdr.e_shentsize = sizeof(Elf32_Shdr);
    if (machine_type == EM_ARM) {
      ehdr.e_flags = 0x5000200;
      ehdr.e_phnum = 2;
    }
    memory_->SetMemory(0, &ehdr, sizeof(ehdr));

    Elf32_Phdr phdr;
    memset(&phdr, 0, sizeof(phdr));
    phdr.p_type = PT_LOAD;
    phdr.p_filesz = 0x10000;
    phdr.p_memsz = 0x10000;
    phdr.p_flags = PF_R | PF_X;
    phdr.p_align = 0x1000;
    memory_->SetMemory(0x100, &phdr, sizeof(phdr));

    if (machine_type == EM_ARM) {
      memset(&phdr, 0, sizeof(phdr));
      phdr.p_type = PT_ARM_EXIDX;
      phdr.p_offset = 0x30000;
      phdr.p_vaddr = 0x30000;
      phdr.p_paddr = 0x30000;
      phdr.p_filesz = 16;
      phdr.p_memsz = 16;
      phdr.p_flags = PF_R;
      phdr.p_align = 0x4;
      memory_->SetMemory(0x100 + sizeof(phdr), &phdr, sizeof(phdr));
    }
  }

  void InitElf64(uint32_t machine_type) {
    Elf64_Ehdr ehdr;
    TestInitEhdr<Elf64_Ehdr>(&ehdr, ELFCLASS64, machine_type);

    ehdr.e_phoff = 0x100;
    ehdr.e_flags = 0x5000200;
    ehdr.e_ehsize = sizeof(ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = 1;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    memory_->SetMemory(0, &ehdr, sizeof(ehdr));

    Elf64_Phdr phdr;
    memset(&phdr, 0, sizeof(phdr));
    phdr.p_type = PT_LOAD;
    phdr.p_filesz = 0x10000;
    phdr.p_memsz = 0x10000;
    phdr.p_flags = PF_R | PF_X;
    phdr.p_align = 0x1000;
    memory_->SetMemory(0x100, &phdr, sizeof(phdr));
  }

  void VerifyStepIfSignalHandler(uint64_t load_bias);

  MemoryFake* memory_;
};

TEST_F(ElfTest, invalid_memory) {
  Elf elf(memory_);

  ASSERT_FALSE(elf.Init());
  ASSERT_FALSE(elf.valid());
}

TEST_F(ElfTest, elf_invalid) {
  Elf elf(memory_);

  InitElf32(EM_386);

  // Corrupt the ELF signature.
  memory_->SetData32(0, 0x7f000000);

  ASSERT_FALSE(elf.Init());
  ASSERT_FALSE(elf.valid());
  ASSERT_TRUE(elf.interface() == nullptr);

  ASSERT_EQ("", elf.GetSoname());

  SharedString name;
  uint64_t func_offset;
  ASSERT_FALSE(elf.GetFunctionName(0, &name, &func_offset));

  ASSERT_FALSE(elf.StepIfSignalHandler(0, nullptr, nullptr));
  EXPECT_EQ(ERROR_INVALID_ELF, elf.GetLastErrorCode());

  bool finished;
  bool is_signal_frame;
  ASSERT_FALSE(elf.Step(0, nullptr, nullptr, &finished, &is_signal_frame));
  EXPECT_EQ(ERROR_INVALID_ELF, elf.GetLastErrorCode());
}

TEST_F(ElfTest, elf_invalid_check_error_values) {
  ElfFake elf(memory_);
  elf.FakeSetValid(false);

  EXPECT_EQ(ERROR_INVALID_ELF, elf.GetLastErrorCode());
  EXPECT_EQ(0U, elf.GetLastErrorAddress());

  ErrorData error = {};
  elf.GetLastError(&error);
  EXPECT_EQ(ERROR_INVALID_ELF, error.code);
  EXPECT_EQ(0U, error.address);

  error.code = ERROR_MEMORY_INVALID;
  error.address = 0x100;
  elf.GetLastError(&error);
  EXPECT_EQ(ERROR_INVALID_ELF, error.code);
  EXPECT_EQ(0U, error.address);
}

TEST_F(ElfTest, elf32_invalid_machine) {
  Elf elf(memory_);

  InitElf32(EM_PPC);

  ResetLogs();
  ASSERT_FALSE(elf.Init());

  ASSERT_EQ("", GetFakeLogBuf());
  ASSERT_EQ("", GetFakeLogPrint());
}

TEST_F(ElfTest, elf64_invalid_machine) {
  Elf elf(memory_);

  InitElf64(EM_PPC64);

  ResetLogs();
  ASSERT_FALSE(elf.Init());

  ASSERT_EQ("", GetFakeLogBuf());
  ASSERT_EQ("", GetFakeLogPrint());
}

TEST_F(ElfTest, elf_arm) {
  Elf elf(memory_);

  InitElf32(EM_ARM);

  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.valid());
  ASSERT_EQ(static_cast<uint32_t>(EM_ARM), elf.machine_type());
  ASSERT_EQ(ELFCLASS32, elf.class_type());
  ASSERT_TRUE(elf.interface() != nullptr);
}

TEST_F(ElfTest, elf_mips) {
  Elf elf(memory_);

  InitElf32(EM_MIPS);

  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.valid());
  ASSERT_EQ(static_cast<uint32_t>(EM_MIPS), elf.machine_type());
  ASSERT_EQ(ELFCLASS32, elf.class_type());
  ASSERT_TRUE(elf.interface() != nullptr);
}

TEST_F(ElfTest, elf_x86) {
  Elf elf(memory_);

  InitElf32(EM_386);

  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.valid());
  ASSERT_EQ(static_cast<uint32_t>(EM_386), elf.machine_type());
  ASSERT_EQ(ELFCLASS32, elf.class_type());
  ASSERT_TRUE(elf.interface() != nullptr);
}

TEST_F(ElfTest, elf_arm64) {
  Elf elf(memory_);

  InitElf64(EM_AARCH64);

  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.valid());
  ASSERT_EQ(static_cast<uint32_t>(EM_AARCH64), elf.machine_type());
  ASSERT_EQ(ELFCLASS64, elf.class_type());
  ASSERT_TRUE(elf.interface() != nullptr);
}

TEST_F(ElfTest, elf_x86_64) {
  Elf elf(memory_);

  InitElf64(EM_X86_64);

  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.valid());
  ASSERT_EQ(static_cast<uint32_t>(EM_X86_64), elf.machine_type());
  ASSERT_EQ(ELFCLASS64, elf.class_type());
  ASSERT_TRUE(elf.interface() != nullptr);
}

TEST_F(ElfTest, elf_mips64) {
  Elf elf(memory_);

  InitElf64(EM_MIPS);

  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.valid());
  ASSERT_EQ(static_cast<uint32_t>(EM_MIPS), elf.machine_type());
  ASSERT_EQ(ELFCLASS64, elf.class_type());
  ASSERT_TRUE(elf.interface() != nullptr);
}

TEST_F(ElfTest, gnu_debugdata_init32) {
  TestInitGnuDebugdata<Elf32_Ehdr, Elf32_Shdr>(ELFCLASS32, EM_ARM, true,
                                               [&](uint64_t offset, const void* ptr, size_t size) {
                                                 memory_->SetMemory(offset, ptr, size);
                                               });

  Elf elf(memory_);
  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.interface() != nullptr);
  ASSERT_TRUE(elf.gnu_debugdata_interface() != nullptr);
  EXPECT_EQ(0x1acU, elf.interface()->gnu_debugdata_offset());
  EXPECT_EQ(0x8cU, elf.interface()->gnu_debugdata_size());
}

TEST_F(ElfTest, gnu_debugdata_init64) {
  TestInitGnuDebugdata<Elf64_Ehdr, Elf64_Shdr>(ELFCLASS64, EM_AARCH64, true,
                                               [&](uint64_t offset, const void* ptr, size_t size) {
                                                 memory_->SetMemory(offset, ptr, size);
                                               });

  Elf elf(memory_);
  ASSERT_TRUE(elf.Init());
  ASSERT_TRUE(elf.interface() != nullptr);
  ASSERT_TRUE(elf.gnu_debugdata_interface() != nullptr);
  EXPECT_EQ(0x200U, elf.interface()->gnu_debugdata_offset());
  EXPECT_EQ(0x90U, elf.interface()->gnu_debugdata_size());
}

TEST_F(ElfTest, rel_pc) {
  ElfFake elf(memory_);

  ElfInterfaceFake* interface = new ElfInterfaceFake(memory_);
  elf.FakeSetInterface(interface);

  elf.FakeSetValid(true);
  MapInfo map_info(nullptr, nullptr, 0x1000, 0x2000, 0, 0, "");

  ASSERT_EQ(0x101U, elf.GetRelPc(0x1101, &map_info));

  elf.FakeSetValid(false);
  ASSERT_EQ(0x101U, elf.GetRelPc(0x1101, &map_info));
}

void ElfTest::VerifyStepIfSignalHandler(uint64_t load_bias) {
  ElfFake elf(memory_);

  RegsArm regs;
  regs[13] = 0x50000;
  regs[15] = 0x8000;

  ElfInterfaceFake* interface = new ElfInterfaceFake(memory_);
  elf.FakeSetInterface(interface);
  elf.FakeSetLoadBias(load_bias);

  memory_->SetData32(0x3000, 0xdf0027ad);
  MemoryFake process_memory;
  process_memory.SetData32(0x50000, 0);
  for (size_t i = 0; i < 16; i++) {
    process_memory.SetData32(0x500a0 + i * sizeof(uint32_t), i);
  }

  elf.FakeSetValid(true);
  ASSERT_TRUE(elf.StepIfSignalHandler(0x3000 + load_bias, &regs, &process_memory));
  EXPECT_EQ(ERROR_NONE, elf.GetLastErrorCode());
  EXPECT_EQ(15U, regs.pc());
  EXPECT_EQ(13U, regs.sp());
}

TEST_F(ElfTest, step_in_signal_map) {
  VerifyStepIfSignalHandler(0);
}

TEST_F(ElfTest, step_in_signal_map_non_zero_load_bias) {
  VerifyStepIfSignalHandler(0x1000);
}

class ElfInterfaceMock : public ElfInterface {
 public:
  ElfInterfaceMock(Memory* memory) : ElfInterface(memory) {}
  virtual ~ElfInterfaceMock() = default;

  bool Init(int64_t*) override { return false; }
  void InitHeaders() override {}
  std::string GetSoname() override { return ""; }
  bool GetFunctionName(uint64_t, SharedString*, uint64_t*) override { return false; }
  std::string GetBuildID() override { return ""; }

  MOCK_METHOD(bool, Step, (uint64_t, Regs*, Memory*, bool*, bool*), (override));
  MOCK_METHOD(bool, GetGlobalVariable, (const std::string&, uint64_t*), (override));
  MOCK_METHOD(bool, IsValidPc, (uint64_t), (override));

  void MockSetDataOffset(uint64_t offset) { data_offset_ = offset; }
  void MockSetDataVaddrStart(uint64_t vaddr) { data_vaddr_start_ = vaddr; }
  void MockSetDataVaddrEnd(uint64_t vaddr) { data_vaddr_end_ = vaddr; }

  void MockSetDynamicOffset(uint64_t offset) { dynamic_offset_ = offset; }
  void MockSetDynamicVaddrStart(uint64_t vaddr) { dynamic_vaddr_start_ = vaddr; }
  void MockSetDynamicVaddrEnd(uint64_t vaddr) { dynamic_vaddr_end_ = vaddr; }
};

TEST_F(ElfTest, step_in_interface) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);

  RegsArm regs;

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);
  MemoryFake process_memory;

  bool finished;
  bool is_signal_frame;
  EXPECT_CALL(*interface, Step(0x1000, &regs, &process_memory, &finished, &is_signal_frame))
      .WillOnce(::testing::Return(true));

  ASSERT_TRUE(elf.Step(0x1000, &regs, &process_memory, &finished, &is_signal_frame));
}

TEST_F(ElfTest, get_global_invalid_elf) {
  ElfFake elf(memory_);
  elf.FakeSetValid(false);

  std::string global("something");
  uint64_t offset;
  ASSERT_FALSE(elf.GetGlobalVariableOffset(global, &offset));
}

TEST_F(ElfTest, get_global_valid_not_in_interface) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);

  std::string global("something");
  EXPECT_CALL(*interface, GetGlobalVariable(global, ::testing::_))
      .WillOnce(::testing::Return(false));

  uint64_t offset;
  ASSERT_FALSE(elf.GetGlobalVariableOffset(global, &offset));
}

TEST_F(ElfTest, get_global_vaddr_in_no_sections) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);

  std::string global("something");
  EXPECT_CALL(*interface, GetGlobalVariable(global, ::testing::_))
      .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(0x300), ::testing::Return(true)));

  uint64_t offset;
  ASSERT_FALSE(elf.GetGlobalVariableOffset(global, &offset));
}

TEST_F(ElfTest, get_global_vaddr_in_data_section) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);
  interface->MockSetDataVaddrStart(0x500);
  interface->MockSetDataVaddrEnd(0x600);
  interface->MockSetDataOffset(0xa000);

  std::string global("something");
  EXPECT_CALL(*interface, GetGlobalVariable(global, ::testing::_))
      .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(0x580), ::testing::Return(true)));

  uint64_t offset;
  ASSERT_TRUE(elf.GetGlobalVariableOffset(global, &offset));
  EXPECT_EQ(0xa080U, offset);
}

TEST_F(ElfTest, get_global_vaddr_in_dynamic_section) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);
  interface->MockSetDataVaddrStart(0x500);
  interface->MockSetDataVaddrEnd(0x600);
  interface->MockSetDataOffset(0xa000);

  interface->MockSetDynamicVaddrStart(0x800);
  interface->MockSetDynamicVaddrEnd(0x900);
  interface->MockSetDynamicOffset(0xc000);

  std::string global("something");
  EXPECT_CALL(*interface, GetGlobalVariable(global, ::testing::_))
      .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(0x880), ::testing::Return(true)));

  uint64_t offset;
  ASSERT_TRUE(elf.GetGlobalVariableOffset(global, &offset));
  EXPECT_EQ(0xc080U, offset);
}

TEST_F(ElfTest, get_global_vaddr_with_tagged_pointer) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);
  elf.FakeSetArch(ARCH_ARM64);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);
  interface->MockSetDataVaddrStart(0x500);
  interface->MockSetDataVaddrEnd(0x600);
  interface->MockSetDataOffset(0xa000);

  std::string global("something");
  EXPECT_CALL(*interface, GetGlobalVariable(global, ::testing::_))
      .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(0x8800000000000580),
                                 ::testing::Return(true)));

  uint64_t offset;
  ASSERT_TRUE(elf.GetGlobalVariableOffset(global, &offset));
  EXPECT_EQ(0xa080U, offset);
}

TEST_F(ElfTest, get_global_vaddr_without_tagged_pointer) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);
  elf.FakeSetArch(ARCH_X86_64);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);
  interface->MockSetDataVaddrStart(0x8800000000000500);
  interface->MockSetDataVaddrEnd(0x8800000000000600);
  interface->MockSetDataOffset(0x880000000000a000);

  std::string global("something");
  EXPECT_CALL(*interface, GetGlobalVariable(global, ::testing::_))
      .WillOnce(::testing::DoAll(::testing::SetArgPointee<1>(0x8800000000000580),
                                 ::testing::Return(true)));

  uint64_t offset;
  ASSERT_TRUE(elf.GetGlobalVariableOffset(global, &offset));
  EXPECT_EQ(0x880000000000a080U, offset);
}

TEST_F(ElfTest, is_valid_pc_elf_invalid) {
  ElfFake elf(memory_);
  elf.FakeSetValid(false);

  EXPECT_FALSE(elf.IsValidPc(0x100));
  EXPECT_FALSE(elf.IsValidPc(0x200));
}

TEST_F(ElfTest, is_valid_pc_interface) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);

  EXPECT_CALL(*interface, IsValidPc(0x1500)).WillOnce(::testing::Return(true));

  EXPECT_TRUE(elf.IsValidPc(0x1500));
}

TEST_F(ElfTest, is_valid_pc_from_gnu_debugdata) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);

  ElfInterfaceMock* interface = new ElfInterfaceMock(memory_);
  elf.FakeSetInterface(interface);
  ElfInterfaceMock* gnu_interface = new ElfInterfaceMock(memory_);
  elf.FakeSetGnuDebugdataInterface(gnu_interface);

  EXPECT_CALL(*interface, IsValidPc(0x1500)).WillOnce(::testing::Return(false));
  EXPECT_CALL(*gnu_interface, IsValidPc(0x1500)).WillOnce(::testing::Return(true));

  EXPECT_TRUE(elf.IsValidPc(0x1500));
}

TEST_F(ElfTest, error_code_valid) {
  ElfFake elf(memory_);
  elf.FakeSetValid(true);
  ElfInterfaceFake* interface = new ElfInterfaceFake(memory_);
  elf.FakeSetInterface(interface);
  interface->FakeSetErrorCode(ERROR_MEMORY_INVALID);
  interface->FakeSetErrorAddress(0x1000);

  ErrorData error{ERROR_NONE, 0};
  elf.GetLastError(&error);
  EXPECT_EQ(ERROR_MEMORY_INVALID, error.code);
  EXPECT_EQ(0x1000U, error.address);
  EXPECT_EQ(ERROR_MEMORY_INVALID, elf.GetLastErrorCode());
  EXPECT_EQ(0x1000U, elf.GetLastErrorAddress());
}

}  // namespace unwindstack
