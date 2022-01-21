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

#include "PeCoffUnwindInfoUnwinderX86_64.h"

#include <algorithm>
#include <limits>

#include "unwindstack/Error.h"
#include "unwindstack/MachineX86_64.h"
#include "unwindstack/RegsX86_64.h"
#include "utils/MemoryFake.h"

#include "Check.h"

#include <gtest/gtest.h>

namespace unwindstack {

class PeCoffUnwindInfoUnwinderX86_64Test : public ::testing::Test {
 public:
  PeCoffUnwindInfoUnwinderX86_64Test()
      : unwinder_(new PeCoffMemory(new MemoryFake)), process_mem_fake_(new MemoryFake) {}
  ~PeCoffUnwindInfoUnwinderX86_64Test() {}

  // See
  // https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-160#operation-info
  enum UnwindInfoRegister : uint8_t {
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

  // These only show up in UWOP_SAVE_XMM128 and UWOP_SAVE_XMM128_FAR operations (which we
  // skip during unwinding) and they are stored as the high 4 bits of a uint8_t in UnwindCode (the
  // "op info"), so even though the regular numbering for these would be 16 to 31, we can only use
  // numbers representable in 4 bits. (The specification is not explicit about how these are
  // represented in op info).
  enum UnwindInfoXmmRegister : uint8_t {
    XMM1 = 0,
    XMM2 = 1,
    XMM3 = 2,
    XMM4 = 3,
    XMM5 = 4,
    XMM6 = 5,
    XMM7 = 6,
    XMM8 = 7,
    XMM9 = 8,
    XMM10 = 9,
    XMM11 = 10,
    XMM12 = 11,
    XMM13 = 12,
    XMM14 = 13,
    XMM15 = 14,
    XMM16 = 15,
  };

  // Only non-volatile registers should be used for these:
  // RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15
  struct PushOp {
    UnwindInfoRegister reg;
    uint64_t value;
  };

  struct SaveOp {
    UnwindInfoRegister reg;
    uint64_t value;
    uint32_t offset;
  };

  // The unwinder doesn't do anything with these operations, so we
  // only have to supply minimal data of what these should look like.
  struct SaveXmm128Op {
    UnwindInfoXmmRegister xmm_reg;
    uint32_t offset;
  };

  struct StackFrameOptions {
    uint64_t return_address = 0;
    uint32_t stack_allocation = 0;

    bool use_frame_pointer = false;
    UnwindInfoRegister frame_pointer_register = RBP;
    uint8_t frame_pointer_offset = 0;
  };

  uint8_t PackUnwindOpAndOpInfo(UnwindOpCode op_code, uint8_t op_info) {
    CHECK((op_code & 0x0f) == op_code);
    CHECK((op_info & 0x0f) == op_info);
    return op_code | (op_info << 4);
  }

  uint8_t PackFrameRegisterAndOffset(UnwindInfoRegister reg, uint8_t offset) {
    CHECK((reg & 0x0f) == reg);
    CHECK(offset <= 240);
    CHECK(offset % 16 == 0);

    uint8_t scaled_offset = offset / 16;
    return reg | (scaled_offset << 4);
  }

  void PackUInt32AsFrameOffsets(uint32_t value, UnwindCode* low_bits, UnwindCode* high_bits) {
    uint16_t value_low_bits = static_cast<uint16_t>(value & 0x0000ffff);
    uint16_t value_high_bits = static_cast<uint16_t>(value >> 16);

    low_bits->frame_offset = value_low_bits;
    high_bits->frame_offset = value_high_bits;
  }

  void AddStackAllocation(uint32_t stack_allocation, std::vector<UnwindCode>* unwind_codes) {
    CHECK(stack_allocation % 8 == 0);
    if (stack_allocation <= 8) {
      return;
    }

    stack_pointer_ -= stack_allocation;
    // We zero out the fake memory corresponding to the memory allocation. This
    // turns all addresses in the area of the allocation valid.
    process_mem_fake_->SetMemoryBlock(stack_pointer_, stack_allocation, 0x00);

    if (stack_allocation <= 128) {
      UnwindCode unwind_code;
      // This value would need to be the address of the instruction after the instruction
      // corresponding to the current unwind operation. Since we don't care about correctness of the
      // number of bytes here, we just use the number of unwind codes + 1. (The "+1" is needed so
      // that an offset of 0 really corresponds to no instructions executed in the current
      // function).
      unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;
      uint8_t op_info = (stack_allocation - 8) / 8;
      unwind_code.code_and_op.unwind_op_and_op_info =
          PackUnwindOpAndOpInfo(UWOP_ALLOC_SMALL, op_info);

      unwind_codes->emplace_back(unwind_code);
    } else if (stack_allocation <= 8 * 65535) {
      UnwindCode unwind_code;
      unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;
      // A value of zero here indicates a large allocation up to 8 * (2^16 - 1).
      uint8_t op_info = 0x00;
      unwind_code.code_and_op.unwind_op_and_op_info =
          PackUnwindOpAndOpInfo(UWOP_ALLOC_LARGE, op_info);

      UnwindCode allocation_size;
      allocation_size.frame_offset = static_cast<uint16_t>(stack_allocation / 8);

      // Will be reversed later into the correct order.
      unwind_codes->emplace_back(allocation_size);
      unwind_codes->emplace_back(unwind_code);
    } else {
      UnwindCode unwind_code;
      unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;
      uint8_t op_info = 0x01;
      unwind_code.code_and_op.unwind_op_and_op_info =
          PackUnwindOpAndOpInfo(UWOP_ALLOC_LARGE, op_info);

      UnwindCode allocation_low_bits;
      UnwindCode allocation_high_bits;

      PackUInt32AsFrameOffsets(stack_allocation, &allocation_low_bits, &allocation_high_bits);

      // Will be reversed later into the correct order. Note that the allocation
      // size is stored in the next two nodes after the unwind code in little Endian
      // order, so the low bits must come first (in the final, reversed order).
      unwind_codes->emplace_back(allocation_high_bits);
      unwind_codes->emplace_back(allocation_low_bits);
      unwind_codes->emplace_back(unwind_code);
    }
  }

  void AddPushedRegisters(const std::vector<PushOp>& pushed_registers,
                          std::vector<UnwindCode>* unwind_codes) {
    for (auto& push_op : pushed_registers) {
      stack_pointer_ -= sizeof(uint64_t);
      process_mem_fake_->SetData64(stack_pointer_, push_op.value);

      UnwindCode unwind_code;
      // We don't put a correct value (which would depend on instruction size), as
      // we just have to be able to distinguish between instructions in some way
      // (so we don't want the same value for the code offset).
      unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;

      unwind_code.code_and_op.unwind_op_and_op_info =
          PackUnwindOpAndOpInfo(UWOP_PUSH_NONVOL, push_op.reg);
      unwind_codes->emplace_back(unwind_code);
    }
  }

  void AddSavedRegisters(const std::vector<SaveOp>& saved_registers,
                         std::vector<UnwindCode>* unwind_codes) {
    for (auto& save_op : saved_registers) {
      CHECK(save_op.offset % 8 == 0);
      process_mem_fake_->SetData64(stack_pointer_ + save_op.offset, save_op.value);

      UnwindCode unwind_code;
      unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;

      if (save_op.offset <= 8 * 65535) {
        unwind_code.code_and_op.unwind_op_and_op_info =
            PackUnwindOpAndOpInfo(UWOP_SAVE_NONVOL, save_op.reg);

        UnwindCode offset;
        offset.frame_offset = static_cast<uint16_t>(save_op.offset / 8);
        unwind_codes->emplace_back(offset);
      } else {
        unwind_code.code_and_op.unwind_op_and_op_info =
            PackUnwindOpAndOpInfo(UWOP_SAVE_NONVOL_FAR, save_op.reg);

        UnwindCode offset_high;
        UnwindCode offset_low;
        PackUInt32AsFrameOffsets(save_op.offset, &offset_low, &offset_high);

        // Will be reversed later into the correct order. Note that the offset
        // is stored in the next two nodes after the unwind code in little Endian
        // order, so the low bits must come first (in the final, reversed order).
        unwind_codes->emplace_back(offset_high);
        unwind_codes->emplace_back(offset_low);
      }

      unwind_codes->emplace_back(unwind_code);
    }
  }

  void AddSavedXmm128Registers(const std::vector<SaveXmm128Op>& saved_xmm128_registers,
                               std::vector<UnwindCode>* unwind_codes) {
    for (auto& save_xmm128_op : saved_xmm128_registers) {
      UnwindCode unwind_code;
      if (save_xmm128_op.offset <= 16 * 65535) {
        unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;
        unwind_code.code_and_op.unwind_op_and_op_info =
            PackUnwindOpAndOpInfo(UWOP_SAVE_XMM128, save_xmm128_op.xmm_reg);

        UnwindCode offset;
        offset.frame_offset = static_cast<uint16_t>(save_xmm128_op.offset / 16);
        unwind_codes->emplace_back(offset);
      } else {
        unwind_code.code_and_op.unwind_op_and_op_info =
            PackUnwindOpAndOpInfo(UWOP_SAVE_XMM128_FAR, save_xmm128_op.xmm_reg);

        UnwindCode offset_high;
        UnwindCode offset_low;
        PackUInt32AsFrameOffsets(save_xmm128_op.offset, &offset_low, &offset_high);

        // Will be reversed later into the correct order. Note that the offset
        // is stored in the next two nodes after the unwind code in little Endian
        // order, so the low bits must come first (in the final, reversed order).
        unwind_codes->emplace_back(offset_high);
        unwind_codes->emplace_back(offset_low);
      }
      unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;
      unwind_codes->emplace_back(unwind_code);
    }
  }

  void AddFramePointerRegisterOp(const StackFrameOptions& options,
                                 std::vector<UnwindCode>* unwind_codes) {
    if (!options.use_frame_pointer) {
      return;
    }
    UnwindCode unwind_code;
    unwind_code.code_and_op.code_offset = unwind_codes->size() + 1;
    unwind_code.code_and_op.unwind_op_and_op_info = PackUnwindOpAndOpInfo(UWOP_SET_FPREG, 0x00);
    unwind_codes->emplace_back(unwind_code);
  }

  // Effectively simulates creating a stack frame and executing the prolog of a function, which
  // is given by the data supplied to this function. For example, the return address to be pushed
  // onto the stack, the stack allocation size, and pushed registers are provided. The correspoding
  // unwind info that one can use to unwind the stack frame is created alongside the simulated
  // operations.
  // Does not verify data. For the stack frame to make sense, saved registers must not
  // overwrite locations of pushed registers and must fall into the allocated area.
  void PushStackFrame(const StackFrameOptions& options, const std::vector<PushOp>& pushed_registers,
                      const std::vector<SaveOp>& saved_registers,
                      const std::vector<SaveXmm128Op>& saved_xmm128_registers,
                      UnwindInfo* unwind_info) {
    UnwindInfo new_unwind_info;

    stack_pointer_ -= sizeof(uint64_t);
    process_mem_fake_->SetData64(stack_pointer_, options.return_address);

    std::vector<UnwindCode> unwind_codes;
    AddPushedRegisters(pushed_registers, &unwind_codes);
    AddStackAllocation(options.stack_allocation, &unwind_codes);
    AddSavedRegisters(saved_registers, &unwind_codes);
    AddSavedXmm128Registers(saved_xmm128_registers, &unwind_codes);
    AddFramePointerRegisterOp(options, &unwind_codes);

    // TODO: Chained info.
    uint8_t flags = 0x00;
    uint8_t version_and_flags = flags | 0x01;

    new_unwind_info.version_and_flags = version_and_flags;
    new_unwind_info.prolog_size = unwind_codes.size() + 1;

    // Unwind codes need to be saved in the order that they will be processed by the unwinding code,
    // which is reverse to the order of machine instructions (which we simulate above).
    std::reverse(unwind_codes.begin(), unwind_codes.end());

    CHECK(unwind_codes.size() <= std::numeric_limits<uint8_t>::max());
    new_unwind_info.num_codes = static_cast<uint8_t>(unwind_codes.size());

    new_unwind_info.frame_register_and_offset = 0x00;
    if (options.use_frame_pointer) {
      new_unwind_info.frame_register_and_offset =
          PackFrameRegisterAndOffset(options.frame_pointer_register, options.frame_pointer_offset);
    }

    new_unwind_info.unwind_codes = unwind_codes;
    new_unwind_info.exception_handler_address = 0L;

    // TODO: Chained info.
    new_unwind_info.chained_info = RuntimeFunction{0, 0, 0};

    *unwind_info = new_unwind_info;
  }

 protected:
  PeCoffUnwindInfoUnwinderX86_64 unwinder_;
  std::unique_ptr<MemoryFake> process_mem_fake_;

  // Stack pointer, 16-byte aligned, starting with a large value as we grow downwards
  uint64_t stack_pointer_ = std::numeric_limits<uint64_t>::max() & ~0x0F;
};

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_inconsistent_num_codes) {
  UnwindInfo unwind_info;
  unwind_info.num_codes = 2;

  RegsX86_64 regs;
  constexpr uint64_t kCodeOffset = 0x0;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, kCodeOffset,
                              &frame_pointer, &frame_pointer_used));
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_with_unsupported_machframe_instruction) {
  UnwindCode unwind_code;
  unwind_code.code_and_op.code_offset = 1;
  unwind_code.code_and_op.unwind_op_and_op_info = PackUnwindOpAndOpInfo(UWOP_PUSH_MACHFRAME, 0x00);

  UnwindInfo unwind_info;
  unwind_info.unwind_codes.emplace_back(unwind_code);
  unwind_info.num_codes = unwind_info.unwind_codes.size();

  RegsX86_64 regs;
  constexpr uint64_t kCodeOffset = 0x0;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, kCodeOffset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_UNSUPPORTED);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_with_unsupported_unwind_instruction) {
  UnwindCode unwind_code;
  unwind_code.code_and_op.code_offset = 1;
  unwind_code.code_and_op.unwind_op_and_op_info =
      PackUnwindOpAndOpInfo(static_cast<UnwindOpCode>(6), 0x00);

  UnwindInfo unwind_info;
  unwind_info.unwind_codes.emplace_back(unwind_code);
  unwind_info.num_codes = unwind_info.unwind_codes.size();

  RegsX86_64 regs;
  constexpr uint64_t kCodeOffset = 0x0;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, kCodeOffset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_succeeds_pushed_registers_only) {
  std::vector<PushOp> push_ops;
  push_ops.emplace_back(PushOp{RDI, 0x100});
  push_ops.emplace_back(PushOp{RSI, 0x200});
  push_ops.emplace_back(PushOp{R12, 0x300});

  constexpr uint64_t kReturnAddress = 0x2000;

  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  PushStackFrame(options, push_ops, {}, {}, &unwind_info);

  RegsX86_64 regs;
  // We use unwind_info.prolog_size to unwind using all unwind codes.
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);

  EXPECT_EQ(regs.sp(), stack_pointer_ + 3 * sizeof(uint64_t));
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RDI], 0x100);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RSI], 0x200);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R12], 0x300);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_invalid_memory_in_push_register_operation) {
  std::vector<PushOp> push_ops;
  push_ops.emplace_back(PushOp{RDI, 0x100});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  PushStackFrame(options, push_ops, {}, {}, &unwind_info);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  process_mem_fake_->Clear();

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_MEMORY_INVALID);
  EXPECT_EQ(unwinder_.GetLastError().address, regs.sp());
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_succeeds_small_allocation) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 32;
  UnwindInfo unwind_info;

  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, {}, {}, &unwind_info);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  // We use unwind_info.prolog_size to unwind using all unwind codes.
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_succeeds_large_allocation_op_info_zero) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 1024;
  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, {}, {}, &unwind_info);

  // Make sure we get a large stack allocation op with op info zero in the unwind codes.
  // These ops have 2 nodes (2 elements) in the unwind code sequence.
  EXPECT_EQ(2, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  // We use unwind_info.prolog_size to unwind using all unwind codes.
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test,
       eval_fails_in_large_allocation_opinfo_zero_with_incomplete_opcodes) {
  constexpr uint32_t kAllocation = 1024;
  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, {}, {}, &unwind_info);

  // Remove the last element.
  unwind_info.unwind_codes.pop_back();
  unwind_info.num_codes = unwind_info.unwind_codes.size();

  EXPECT_EQ(unwind_info.unwind_codes.size(), 1);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  // We use unwind_info.prolog_size to unwind using all unwind codes.
  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_succeeds_large_allocation_op_info_one) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 8 * 100 * 1024;
  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, {}, {}, &unwind_info);

  // Make sure we get a large stack allocation op with op info zero in the unwind codes.
  // These ops have 3 nodes (3 elements) in the unwind code sequence.
  EXPECT_EQ(3, unwind_info.num_codes);

  RegsX86_64 regs;
  // We use unwind_info.prolog_size to unwind using all unwind codes.
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test,
       eval_fails_in_large_allocation_opinfo_one_with_incomplete_opcodes) {
  constexpr uint32_t kAllocation = 8 * 100 * 1024;
  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, {}, {}, &unwind_info);

  // Remove the last element.
  unwind_info.unwind_codes.pop_back();
  unwind_info.num_codes = unwind_info.unwind_codes.size();

  EXPECT_EQ(unwind_info.unwind_codes.size(), 2);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  // We use unwind_info.prolog_size to unwind using all unwind codes.
  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test,
       eval_succeeds_stack_allocation_and_saved_registers_only) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 1024;

  std::vector<SaveOp> save_ops;
  save_ops.emplace_back(SaveOp{RDI, 0x100, 0x20});
  save_ops.emplace_back(SaveOp{RSI, 0x200, 0x30});
  save_ops.emplace_back(SaveOp{R12, 0x300, 0x40});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, save_ops, {}, &unwind_info);

  // Must get 2 * 3 save ops slots (each save op takes up two slots) and 2 stack allocation slots.
  EXPECT_EQ(2 * 3 + 2, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Validate saved registers.
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RDI], 0x100);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RSI], 0x200);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R12], 0x300);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_saved_registers_invalid_memory) {
  std::vector<SaveOp> save_ops;
  save_ops.emplace_back(SaveOp{RDI, 0x100, 0x20});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  PushStackFrame(options, {}, save_ops, {}, &unwind_info);

  EXPECT_EQ(2, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);
  process_mem_fake_->Clear();

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_MEMORY_INVALID);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_saved_registers_opcode_incomplete) {
  std::vector<SaveOp> save_ops;
  save_ops.emplace_back(SaveOp{RDI, 0x100, 0x20});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  PushStackFrame(options, {}, save_ops, {}, &unwind_info);

  unwind_info.unwind_codes.pop_back();
  unwind_info.num_codes = unwind_info.unwind_codes.size();

  EXPECT_EQ(1, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer;
  bool frame_pointer_used;

  regs.set_sp(stack_pointer_);

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test,
       eval_succeeds_stack_allocation_and_saved_registers_large_offsets_only) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 8 * 100 * 1024;

  std::vector<SaveOp> save_ops;
  save_ops.emplace_back(SaveOp{RDI, 0x100, kAllocation - 0x20});
  save_ops.emplace_back(SaveOp{RSI, 0x200, kAllocation - 0x30});
  save_ops.emplace_back(SaveOp{R12, 0x300, kAllocation - 0x40});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, save_ops, {}, &unwind_info);

  // Must get 3 * 3 save ops slots (each save op takes up three slots) and 3 stack allocation slots.
  EXPECT_EQ(3 * 3 + 3, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Validate saved registers.
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RDI], 0x100);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RSI], 0x200);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R12], 0x300);

  // Also validate that we skip succeessfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_saved_registers_far_opcode_invalid_memory) {
  constexpr uint32_t kLargeOffset = 8 * 100 * 1024;

  std::vector<SaveOp> save_ops;
  save_ops.emplace_back(SaveOp{RDI, 0x100, kLargeOffset});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  PushStackFrame(options, {}, save_ops, {}, &unwind_info);

  EXPECT_EQ(3, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);
  process_mem_fake_->Clear();

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_MEMORY_INVALID);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_fails_saved_registers_far_opcode_incomplete) {
  constexpr uint32_t kLargeOffset = 8 * 100 * 1024;

  std::vector<SaveOp> save_ops;
  save_ops.emplace_back(SaveOp{RDI, 0x100, kLargeOffset});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  PushStackFrame(options, {}, save_ops, {}, &unwind_info);

  unwind_info.unwind_codes.pop_back();
  unwind_info.num_codes = unwind_info.unwind_codes.size();

  EXPECT_EQ(2, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  EXPECT_FALSE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                              &frame_pointer, &frame_pointer_used));
  EXPECT_EQ(unwinder_.GetLastError().code, ERROR_INVALID_COFF);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test,
       eval_succeeds_stack_allocation_and_saved_xmm128_registers_small_offsets) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 1024;

  std::vector<SaveXmm128Op> save_xmm128_ops;
  save_xmm128_ops.emplace_back(SaveXmm128Op{XMM1, 0x20});
  save_xmm128_ops.emplace_back(SaveXmm128Op{XMM2, 0x30});
  save_xmm128_ops.emplace_back(SaveXmm128Op{XMM3, 0x40});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, {}, save_xmm128_ops, &unwind_info);

  EXPECT_EQ(2 * 3 + 2, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test,
       eval_succeeds_stack_allocation_and_saved_xmm128_registers_large_offsets_only) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 16 * 100 * 1024;

  std::vector<SaveXmm128Op> save_xmm128_ops;
  save_xmm128_ops.emplace_back(SaveXmm128Op{XMM1, kAllocation - 0x20});
  save_xmm128_ops.emplace_back(SaveXmm128Op{XMM2, kAllocation - 0x30});
  save_xmm128_ops.emplace_back(SaveXmm128Op{XMM3, kAllocation - 0x40});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, {}, {}, save_xmm128_ops, &unwind_info);

  // Must get 3 * 3 save ops slots (each save op takes up three slots) and 3 slots for the stack
  // allocation.
  EXPECT_EQ(3 * 3 + 3, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test, eval_succeeds_set_frame_pointer_register) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 32;
  UnwindInfo unwind_info;

  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  options.use_frame_pointer = true;
  // Any register can act as the frame pointer register.
  options.frame_pointer_register = RSI;
  options.frame_pointer_offset = 0x10;
  PushStackFrame(options, {}, {}, {}, &unwind_info);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);
  const uint64_t expected_frame_pointer_value = stack_pointer_ + options.frame_pointer_offset;
  // Make sure the frame pointer register is different from the expected value after the
  // operation is carried out.
  regs[X86_64Reg::X86_64_REG_RSI] = expected_frame_pointer_value + 1;

  // We use unwind_info.prolog_size to unwind using all unwind codes.
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_TRUE(frame_pointer_used);
  EXPECT_EQ(frame_pointer, expected_frame_pointer_value);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RSI], expected_frame_pointer_value);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation);

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Also validate that we skip successfully if code offset is before the unwind op.
  frame_pointer_used = false;
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
}

TEST_F(PeCoffUnwindInfoUnwinderX86_64Test,
       eval_succeeds_stack_allocation_and_pushed_and_saved_registers) {
  constexpr uint64_t kReturnAddress = 0x2000;
  constexpr uint32_t kAllocation = 1024;

  std::vector<PushOp> push_ops;
  push_ops.emplace_back(PushOp{RDI, 0x100});
  push_ops.emplace_back(PushOp{RSI, 0x200});
  push_ops.emplace_back(PushOp{R12, 0x300});

  std::vector<SaveOp> save_ops;
  save_ops.emplace_back(SaveOp{RBX, 0x400, 0x20});
  save_ops.emplace_back(SaveOp{R13, 0x500, 0x30});
  save_ops.emplace_back(SaveOp{R14, 0x600, 0x40});

  UnwindInfo unwind_info;
  StackFrameOptions options;
  options.return_address = kReturnAddress;
  options.stack_allocation = kAllocation;
  PushStackFrame(options, push_ops, save_ops, {}, &unwind_info);

  EXPECT_EQ(3 + 2 * 3 + 2, unwind_info.num_codes);

  RegsX86_64 regs;
  uint64_t code_offset = unwind_info.prolog_size;
  uint64_t frame_pointer = 0;
  bool frame_pointer_used = false;

  regs.set_sp(stack_pointer_);

  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, code_offset,
                             &frame_pointer, &frame_pointer_used));
  EXPECT_FALSE(frame_pointer_used);
  EXPECT_EQ(regs.sp(), stack_pointer_ + kAllocation + 3 * sizeof(uint64_t));

  // Validate that stack pointer points to the return address.
  uint64_t return_address;
  ASSERT_TRUE(process_mem_fake_->Read64(regs.sp(), &return_address));
  EXPECT_EQ(return_address, kReturnAddress);

  // Validate stored registers.
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RDI], 0x100);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RSI], 0x200);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R12], 0x300);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_RBX], 0x400);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R13], 0x500);
  EXPECT_EQ(regs[X86_64Reg::X86_64_REG_R14], 0x600);

  // Also validate that we skip successfully if code offset is before the unwind op.
  regs.set_sp(stack_pointer_);
  EXPECT_TRUE(unwinder_.Eval(process_mem_fake_.get(), &regs, unwind_info, 0, &frame_pointer,
                             &frame_pointer_used));
  EXPECT_EQ(regs.sp(), stack_pointer_);
}

}  // namespace unwindstack