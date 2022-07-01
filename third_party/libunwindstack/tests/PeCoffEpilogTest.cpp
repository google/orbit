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

#include "PeCoffEpilog.h"

#include <array>
#include <cstring>
#include <memory>
#include <vector>

#include <unwindstack/MachineX86_64.h>
#include <unwindstack/RegsX86_64.h>
#include "Check.h"
#include "MemoryFake.h"

#include <gtest/gtest.h>

namespace unwindstack {

// While XMM registers can occur in epilog code (and as UNWIND_INFO codes), they can
// not be pushed to the stack, they are always saved with a 'mov' instruction into the
// area allocated on the stack for the current stack frame. In epilogs, the
// corresponding restore operations do not exist and we therefore do not have to care
// about them here.
enum Register : uint8_t {
  RAX = 0,
  RCX = 1,
  RDX = 2,
  RBX = 3,
  RSP = 4,
  RBP = 5,
  RSI = 6,
  RDI = 7,
  R8 = 8,
  R9 = 9,
  R10 = 10,
  R11 = 11,
  R12 = 12,
  R13 = 13,
  R14 = 14,
  R15 = 15,
};

// Only non-volatile registers should be used for these:
// RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15
struct PopOp {
  Register reg;
  uint64_t value;
};

// Used as a parameter for BuildEpilog below to specify the structure of the
// epilog.
struct EpilogOptions {
  // Return address to be set on the stack.
  uint64_t return_address = 0;

  // Insert 'lea' instruction as the first instruction of the epilog.
  bool insert_lea_instruction = false;
  // The 'lea' instruction is only to be used when a frame pointer register
  // is being used. This sets the frame pointer register to be used.
  Register frame_pointer_register = RBP;
  // Displacement value to be used by the 'lea' instruction.
  uint32_t lea_displacement = 0;
  // Value in the frame pointer register.
  uint64_t frame_pointer_register_value = 0;

  // Insert 'add' instruction as the first instruction of the epilog. This
  // is for deallocating the stack allocation.
  bool insert_add_instruction = false;
  // Must be > 0 when inserting an 'add' instruction/
  int32_t added_value = 0;

  // Sequence of pop instructions to be added into the epilog. These are all
  // the callee saved registers that are saved by the function.
  std::vector<PopOp> pop_operations;

  // Instruction bytes for the final 'jmp' or 'ret' can be fairly diverse,
  // so we just directly specify the bytes in each test and pass them in the
  // BuildEpilog method to insert them at the end of the machine code built up.
  std::vector<uint8_t> jmp_instruction_bytes;
  std::vector<uint8_t> ret_instruction_bytes;
};

class PeCoffEpilogTest : public ::testing::Test {
 public:
  // For all tests, we'll have a minimum setup where the machine code to be tested for being an
  // epilog is exactly the machine code we write into the file at offset kTextSectionFileOffset and
  // there is no machine code above this start address. Since we want to (implicitly) test that the
  // address arithmetic is carried out correctly, which needs to convert from relative virtual
  // addresses to file offsets, we use some non-zero values here.
  static constexpr uint64_t kFunctionStartAddress = 0x1000;
  static constexpr uint64_t kCurrentOffsetFromStartOfFunction = 0;
  static constexpr uint64_t kTextSectionVmaddr = kFunctionStartAddress;
  static constexpr uint64_t kTextSectionFileOffset = 0x100;
  static constexpr uint64_t kTextSectionSize = 0x200;
  static inline const Section kTextSection{
      ".text", kTextSectionSize, kTextSectionVmaddr, kTextSectionSize, kTextSectionFileOffset, 0};

  static constexpr uint64_t kSecondFunctionStartAddress = 0x2000;
  static constexpr uint64_t kSecondTextSectionVmaddr = kSecondFunctionStartAddress;
  static constexpr uint64_t kSecondTextSectionFileOffset = 0x300;
  static constexpr uint64_t kSecondTextSectionSize = 0x400;
  static inline const Section kSecondTextSection{".text2",
                                                 kSecondTextSectionSize,
                                                 kSecondTextSectionVmaddr,
                                                 kSecondTextSectionSize,
                                                 kSecondTextSectionFileOffset,
                                                 0};

  PeCoffEpilogTest()
      : process_mem_fake_(new MemoryFake),
        file_mem_fake_(new MemoryFake),
        pe_coff_epilog_(
            CreatePeCoffEpilog(file_mem_fake_.get(), {kTextSection, kSecondTextSection})) {}
  ~PeCoffEpilogTest() override = default;

  void SetUp() override { ASSERT_TRUE(pe_coff_epilog_->Init()); }

  // Builds machine code according to the desired epilog structure as specified in the 'options'
  // argument. A good source for understanding and validating instruction encoding can be found
  // here: https://wiki.osdev.org/X86-64_Instruction_Encoding. In particular relevant are REX
  // prefix and ModRM encodings. Instruction reference with opcodes can be found on
  // https://www.felixcloutier.com/x86/, or in the official AMD and Intel manuals, which can be
  // found at https://www.amd.com/system/files/TechDocs/24594.pdf and
  // https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
  std::vector<uint8_t> BuildEpilog(const EpilogOptions& options) {
    std::vector<uint8_t> machine_code;
    // Cannot have both a 'lea' and an 'add' to deallocate the stack allocation.
    CHECK(!options.insert_lea_instruction || !options.insert_add_instruction);
    if (options.insert_lea_instruction) {
      // The REX prefix always has the value 0100 WRXB, where RB can be used to
      // modulate the registers used as operands. In this case, if the frame
      // pointer register is one of R8 to R15, then we need to set the B bit to 1.
      uint8_t rex_byte = 0x48;
      if (options.frame_pointer_register >= R8) {
        rex_byte = 0x49;
      }
      machine_code.insert(machine_code.end(), {rex_byte, 0x8d});
      if (options.lea_displacement <= 0xff) {
        // Only want the lower three bits of the register, the highest bit is indicated
        // in the REX prefix.
        uint8_t modrm = 0b01'100'000 | (options.frame_pointer_register & 0b0111);
        machine_code.emplace_back(modrm);
        uint8_t disp = static_cast<uint8_t>(options.lea_displacement);
        machine_code.emplace_back(disp);
      } else {
        // Only want the lower three bits of the register, the highest bit is indicated
        // in the REX prefix.
        uint8_t modrm = 0b10'100'000 | (options.frame_pointer_register & 0b0111);
        machine_code.emplace_back(modrm);
        std::array<uint8_t, 4> values;
        uint32_t lea_displacement = options.lea_displacement;
        std::memcpy(static_cast<void*>(&values[0]), static_cast<void*>(&lea_displacement),
                    sizeof(uint32_t));
        machine_code.insert(machine_code.end(), values.begin(), values.end());
      }
      expected_stack_pointer_after_unwind_ =
          options.frame_pointer_register_value + options.lea_displacement;
    }
    if (options.insert_add_instruction) {
      CHECK(options.added_value > 0);
      machine_code.emplace_back(0x48);
      if (options.added_value <= 127) {
        machine_code.insert(machine_code.end(), {0x83, 0xc4});
        uint8_t immediate = static_cast<uint8_t>(options.added_value);
        machine_code.emplace_back(immediate);
      } else {
        machine_code.insert(machine_code.end(), {0x81, 0xc4});
        std::array<uint8_t, 4> values;
        int32_t added_value = options.added_value;
        std::memcpy(static_cast<void*>(&values[0]), static_cast<void*>(&added_value),
                    sizeof(int32_t));
        machine_code.insert(machine_code.end(), values.begin(), values.end());
      }
      expected_stack_pointer_after_unwind_ += options.added_value;
    }

    for (const auto& pop_op : options.pop_operations) {
      process_mem_fake_->SetData64(expected_stack_pointer_after_unwind_, pop_op.value);
      expected_stack_pointer_after_unwind_ += sizeof(uint64_t);
      // For 'pop' operations, the REX prefix is only used when one of R8 to R15 is
      // the operand. In that case we need to use the fixed value of 0x41 as the
      // REX prefix.
      if (pop_op.reg >= R8) {
        machine_code.emplace_back(0x41);
      }
      // Only want the lower three bits of the register value. If the highest bit is
      // 1, this is indicated by presence of the REX prefix.
      uint8_t opcode_byte = 0x58 | (pop_op.reg & 0x7);
      machine_code.emplace_back(opcode_byte);
    }

    process_mem_fake_->SetData64(expected_stack_pointer_after_unwind_, options.return_address);
    expected_stack_pointer_after_unwind_ += sizeof(uint64_t);

    CHECK(options.jmp_instruction_bytes.empty() != options.ret_instruction_bytes.empty());
    machine_code.insert(machine_code.end(), options.jmp_instruction_bytes.begin(),
                        options.jmp_instruction_bytes.end());
    machine_code.insert(machine_code.end(), options.ret_instruction_bytes.begin(),
                        options.ret_instruction_bytes.end());
    return machine_code;
  }

  void SetMemoryInFakeFile(uint64_t offset, const std::vector<uint8_t>& data) {
    file_mem_fake_->SetMemory(offset, data);
  }

 protected:
  std::unique_ptr<MemoryFake> process_mem_fake_;
  std::unique_ptr<MemoryFake> file_mem_fake_;
  std::unique_ptr<PeCoffEpilog> pe_coff_epilog_;
  // Anything we do in the tests will increase the stack pointer value, so this is a safe starting
  // point.
  uint64_t expected_stack_pointer_after_unwind_ = 0;
};

TEST_F(PeCoffEpilogTest, aborts_on_process_memory_nullptr) {
  RegsX86_64 regs;

  // We need a minimal correct setup, otherwise we might fail due to different reasons than the
  // nullptr.
  constexpr uint64_t kFunctionStartAddress = kTextSectionVmaddr;
  constexpr uint64_t kFunctionEndAddress = kTextSectionVmaddr + 1;
  SetMemoryInFakeFile(kTextSectionFileOffset, {0x0});
  bool is_in_epilog;
  ASSERT_DEATH(pe_coff_epilog_->DetectAndHandleEpilog(kFunctionStartAddress, kFunctionEndAddress, 0,
                                                      nullptr, &regs, &is_in_epilog),
               "");
}

TEST_F(PeCoffEpilogTest, aborts_on_regs_nullptr) {
  // We need a minimal correct setup, otherwise we might fail due to different reasons than the
  // nullptr.
  constexpr uint64_t kFunctionStartAddress = kTextSectionVmaddr;
  constexpr uint64_t kFunctionEndAddress = kTextSectionVmaddr + 1;
  SetMemoryInFakeFile(kTextSectionFileOffset, {0x0});
  bool is_in_epilog;
  ASSERT_DEATH(
      pe_coff_epilog_->DetectAndHandleEpilog(kFunctionStartAddress, kFunctionEndAddress, 0,
                                             process_mem_fake_.get(), nullptr, &is_in_epilog),
      "");
}

TEST_F(PeCoffEpilogTest, fails_if_file_memory_cannot_be_read) {
  RegsX86_64 regs;
  regs.set_sp(0);

  // Don't care about the exact value here, just needs to be > kFunctionStartAddress so that we
  // attempt to read machine code from the file memory (which is purposefully empty to trigger an
  // error).
  constexpr uint64_t kFunctionEndAddressFakeValue = kFunctionStartAddress + 1;

  bool is_in_epilog;
  EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, kFunctionEndAddressFakeValue, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_MEMORY_INVALID);
}

TEST_F(PeCoffEpilogTest, fails_if_end_address_is_smaller_than_start_address) {
  RegsX86_64 regs;
  regs.set_sp(0);

  CHECK(kFunctionStartAddress > 0);
  constexpr uint64_t kFunctionEndAddressFakeValue = kFunctionStartAddress - 1;

  bool is_in_epilog;
  EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, kFunctionEndAddressFakeValue, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffEpilogTest, fails_if_function_start_smaller_than_text_section_start) {
  RegsX86_64 regs;
  regs.set_sp(0);

  constexpr uint64_t kFunctionStartAddress = kTextSectionVmaddr - 1;
  constexpr uint64_t kFunctionEndAddress = kFunctionStartAddress + 1;

  bool is_in_epilog;
  EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, kFunctionEndAddress, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffEpilogTest, fails_if_function_start_larger_than_text_section_end) {
  RegsX86_64 regs;
  regs.set_sp(0);

  constexpr uint64_t kFunctionStartAddress = kTextSectionVmaddr + kTextSectionSize;
  constexpr uint64_t kFunctionEndAddress = kFunctionStartAddress + 1;

  bool is_in_epilog;
  EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, kFunctionEndAddress, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffEpilogTest, fails_if_disassembling_fails) {
  std::vector<uint8_t> machine_code{0x48, 0x81, 0xc1, 0x07,
                                    0xc3};  // bogus machine code, two bytes are missing

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_UNSUPPORTED);
}

TEST_F(PeCoffEpilogTest, fails_if_memory_at_return_address_is_invalid) {
  std::vector<uint8_t> machine_code{0xc3};  // ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_MEMORY_INVALID);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().address, 0);
}

TEST_F(PeCoffEpilogTest, detects_epilog_add_with_small_value_and_ret_only) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.insert_add_instruction = true;
  // Needs to be <= 0xff to trigger the "small value case".
  options.added_value = 0x10;
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
}

TEST_F(PeCoffEpilogTest, detects_epilog_add_with_large_value_and_ret_only) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.insert_add_instruction = true;
  // Needs to be > 0xff to trigger the "large value case".
  options.added_value = 0x1000;
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_missing_ret_instruction) {
  std::vector<uint8_t> machine_code{0x48, 0x83, 0xc4, 0x28};  // add sp, 0x28

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_add_instruction_not_rsp) {
  std::vector<uint8_t> machine_code{0x48, 0x83, 0xc1, 0x07, 0xc3};  // add rcx, 0x7; ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_add_instruction_not_immediate_added_to_rsp) {
  std::vector<uint8_t> machine_code{0x48, 0x01, 0xc4, 0xc3};  // add rsp, rax; ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_add_instruction_destination_not_register) {
  std::vector<uint8_t> machine_code{0x48, 0x01, 0x04, 0x24, 0xc3};  // add [rsp], rax; ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_add_instruction_immediate_negative) {
  // The immediate value represents the stack allocation size, so must be non-negative.
  std::vector<uint8_t> machine_code{0x48, 0x83, 0xc4, 0xff, 0xc3};  // add rsp, -1; ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_lea_with_small_displacement_and_ret_only) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.insert_lea_instruction = true;
  // Needs to be <= 0xff to trigger the "small displacement" case.
  options.lea_displacement = 0x20;
  options.frame_pointer_register_value = 0x1000;
  options.frame_pointer_register = RBP;
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);
  regs[X86_64Reg::X86_64_REG_RBP] = options.frame_pointer_register_value;

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
}

TEST_F(PeCoffEpilogTest, detects_epilog_lea_with_large_displacement_and_ret_only) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.insert_lea_instruction = true;
  // Needs to be > 0xff to trigger the "large displacement" case.
  options.lea_displacement = 0x100;
  options.frame_pointer_register_value = 0x1000;
  options.frame_pointer_register = RBP;
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);
  regs[X86_64Reg::X86_64_REG_RBP] = options.frame_pointer_register_value;

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_instruction_lea_destination_is_not_rsp) {
  std::vector<uint8_t> machine_code{0x48, 0x8d, 0x75, 0x00, 0xc3};  // lea rsi,[rbp+0x0]; ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_instruction_lea_second_operand_is_not_base_plus_value) {
  // lea rsp,[rbp+rax*2+0x2]
  std::vector<uint8_t> machine_code{0x48, 0x8d, 0x64, 0x45, 0x02, 0xc3};

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_pop_instructions_and_ret_only) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.pop_operations = {PopOp{RSI, 0x100}, PopOp{R12, 0x200}, PopOp{RBX, 0x300},
                            PopOp{R11, 0x400}};
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);
  regs[X86_64Reg::X86_64_REG_RSI] = 0;
  regs[X86_64Reg::X86_64_REG_R12] = 0;
  regs[X86_64Reg::X86_64_REG_RBX] = 0;
  regs[X86_64Reg::X86_64_REG_R11] = 0;

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RSI], 0x100);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R12], 0x200);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RBX], 0x300);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R11], 0x400);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_pop_to_memory) {
  std::vector<uint8_t> machine_code{0x8f, 0x41, 0x70, 0xc3};  // pop QWORD PTR [rcx+0x70]; ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_pop_to_two_byte_register) {
  std::vector<uint8_t> machine_code{0x66, 0x5e, 0xc3};  // pop si; ret

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, fails_if_invalid_memory_on_register_store_location) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.pop_operations = {PopOp{RSI, 0x100}};
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  // This is where RSI is stored, clear it so that we run into the error case.
  process_mem_fake_->ClearMemory(0, sizeof(uint64_t));

  bool is_in_epilog;
  EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_MEMORY_INVALID);
}

TEST_F(PeCoffEpilogTest, detects_epilog_near_return) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_near_return_with_immediate) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.ret_instruction_bytes = {0xc2, 0x01, 0x02};  // ret 0x201

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_far_return) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.ret_instruction_bytes = {0xcb};  // retf

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_far_return_with_immediate) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.ret_instruction_bytes = {0xca, 0x01, 0x02};  // retf 0x201

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_jmp_ff) {
  EpilogOptions options;
  options.return_address = 0x1234;
  // jmp    QWORD PTR [rip+0x918ea]
  options.jmp_instruction_bytes = {0xff, 0x25, 0xea, 0x18, 0x09, 0x00};

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_jmp_with_rex_prefix) {
  EpilogOptions options;
  options.return_address = 0x1234;
  // rex.W jmp QWORD PTR [rip+0x126ced]
  options.jmp_instruction_bytes = {0x48, 0xff, 0x25, 0xed, 0x6c, 0x12, 0x00};

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_TRUE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_jmp_wrong_modrm_byte) {
  EpilogOptions options;
  options.return_address = 0x1234;
  // modrm.mod is 01 in this case, which should be rejected
  // jmp    QWORD PTR [rbp-0x16]
  options.jmp_instruction_bytes = {0xff, 0x65, 0xea};

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool is_in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &is_in_epilog));
  EXPECT_FALSE(is_in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_non_epilog_jmp_no_memory_reference) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.jmp_instruction_bytes = {0xeb, 0x01};  // jmp 0x3

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(kFunctionStartAddress, function_end_address,
                                                     kCurrentOffsetFromStartOfFunction,
                                                     process_mem_fake_.get(), &regs, &in_epilog));
  EXPECT_FALSE(in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
}

TEST_F(PeCoffEpilogTest, detects_epilog_general_case_with_lea_as_first_instruction) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.insert_lea_instruction = true;
  options.lea_displacement = 0x20;
  options.frame_pointer_register_value = 0x1000;
  options.frame_pointer_register = RBP;
  options.pop_operations = {PopOp{RDI, 0x100}, PopOp{R12, 0x200}, PopOp{RBX, 0x300}};
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);
  regs[X86_64Reg::X86_64_REG_RBP] = options.frame_pointer_register_value;
  regs[X86_64Reg::X86_64_REG_RDI] = 0;
  regs[X86_64Reg::X86_64_REG_R12] = 0;
  regs[X86_64Reg::X86_64_REG_RBX] = 0;

  bool in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(kFunctionStartAddress, function_end_address,
                                                     kCurrentOffsetFromStartOfFunction,
                                                     process_mem_fake_.get(), &regs, &in_epilog));
  EXPECT_TRUE(in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RDI], 0x100);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R12], 0x200);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RBX], 0x300);
}

TEST_F(PeCoffEpilogTest, detects_epilog_general_case_with_add_as_first_instruction) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.insert_add_instruction = true;
  options.added_value = 0x1000;
  options.pop_operations = {PopOp{RDI, 0x100}, PopOp{R12, 0x200}, PopOp{RBX, 0x300}};
  options.ret_instruction_bytes = {0xc3};  // ret

  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);
  regs[X86_64Reg::X86_64_REG_RDI] = 0;
  regs[X86_64Reg::X86_64_REG_R12] = 0;
  regs[X86_64Reg::X86_64_REG_RBX] = 0;

  bool in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(kFunctionStartAddress, function_end_address,
                                                     kCurrentOffsetFromStartOfFunction,
                                                     process_mem_fake_.get(), &regs, &in_epilog));
  EXPECT_TRUE(in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RDI], 0x100);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R12], 0x200);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RBX], 0x300);
}

TEST_F(PeCoffEpilogTest, succeeds_with_pc_not_in_first_executable_section) {
  EpilogOptions options;
  options.return_address = 0x1234;
  options.ret_instruction_bytes = {0xc3};  // ret
  std::vector<uint8_t> machine_code = BuildEpilog(options);

  SetMemoryInFakeFile(kSecondTextSectionFileOffset, machine_code);
  const uint64_t function_end_address = kSecondFunctionStartAddress + machine_code.size();

  RegsX86_64 regs;
  regs.set_sp(0);

  bool in_epilog;
  EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
      kSecondFunctionStartAddress, function_end_address, kCurrentOffsetFromStartOfFunction,
      process_mem_fake_.get(), &regs, &in_epilog));
  EXPECT_TRUE(in_epilog);
  EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);

  EXPECT_EQ(regs.pc(), options.return_address);
  EXPECT_EQ(regs.sp(), expected_stack_pointer_after_unwind_);
}

TEST_F(PeCoffEpilogTest, error_is_reset_for_every_invocation) {
  {
    // Cannot read memory.
    RegsX86_64 regs;
    constexpr uint64_t kFunctionEndAddressFakeValue = kFunctionStartAddress + 1;
    bool in_epilog;
    EXPECT_FALSE(pe_coff_epilog_->DetectAndHandleEpilog(
        kFunctionStartAddress, kFunctionEndAddressFakeValue, kCurrentOffsetFromStartOfFunction,
        process_mem_fake_.get(), &regs, &in_epilog));
    EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_MEMORY_INVALID);
  }

  {
    // No ret instruction.
    std::vector<uint8_t> machine_code{0x48, 0x83, 0xc4, 0x28};  // add sp, 0x28
    SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
    const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

    RegsX86_64 regs;
    bool in_epilog;
    EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(kFunctionStartAddress, function_end_address,
                                                       kCurrentOffsetFromStartOfFunction,
                                                       process_mem_fake_.get(), &regs, &in_epilog));
    EXPECT_FALSE(in_epilog);
    EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
  }
}

TEST_F(PeCoffEpilogTest, cache_of_addresses_not_in_epilog_can_prevent_failure) {
  {
    // No ret instruction.
    std::vector<uint8_t> machine_code{0x48, 0x83, 0xc4, 0x28};  // add sp, 0x28
    SetMemoryInFakeFile(kTextSectionFileOffset, machine_code);
    const uint64_t function_end_address = kFunctionStartAddress + machine_code.size();

    RegsX86_64 regs;
    bool in_epilog;
    EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(kFunctionStartAddress, function_end_address,
                                                       kCurrentOffsetFromStartOfFunction,
                                                       process_mem_fake_.get(), &regs, &in_epilog));
    EXPECT_FALSE(in_epilog);
    EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
  }

  {
    // Cannot read memory.
    file_mem_fake_->Clear();
    RegsX86_64 regs;
    constexpr uint64_t kFunctionEndAddressFakeValue = kFunctionStartAddress + 1;
    bool in_epilog;
    EXPECT_TRUE(pe_coff_epilog_->DetectAndHandleEpilog(
        kFunctionStartAddress, kFunctionEndAddressFakeValue, kCurrentOffsetFromStartOfFunction,
        process_mem_fake_.get(), &regs, &in_epilog));
    EXPECT_FALSE(in_epilog);
    EXPECT_EQ(pe_coff_epilog_->GetLastError().code, ERROR_NONE);
  }
}

}  // namespace unwindstack