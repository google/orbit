// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionFramepointerValidator.h"

#include <capstone/capstone.h>
#include <iostream>

FunctionFramepointerValidator::FunctionFramepointerValidator(
    csh handle, const uint8_t* code, size_t code_size) {
  handle_ = handle;
  instructions_ = nullptr;
  instructions_count_ =
      cs_disasm(handle, code, code_size, 0, 0, &instructions_);
}

FunctionFramepointerValidator::~FunctionFramepointerValidator() {
  cs_free(instructions_, instructions_count_);
}

bool FunctionFramepointerValidator::IsCallInstruction(
    const cs_insn& instruction) {
  for (uint8_t i = 0; i < instruction.detail->groups_count; i++) {
    if (instruction.detail->groups[i] == X86_GRP_CALL) return true;
  }
  return false;
}

bool FunctionFramepointerValidator::IsRetOrJumpInstruction(
    const cs_insn& instruction) {
  for (uint8_t i = 0; i < instruction.detail->groups_count; i++) {
    if (instruction.detail->groups[i] == X86_GRP_RET ||
        instruction.detail->groups[i] == X86_GRP_JUMP)
      return true;
  }
  return false;
}

bool FunctionFramepointerValidator::IsMovInstruction(
    const cs_insn& instruction) {
  return instruction.id == X86_INS_MOV || instruction.id == X86_INS_MOVQ;
}

bool FunctionFramepointerValidator::IsBasePointer(uint16_t reg) {
  return reg == X86_REG_BP || reg == X86_REG_EBP || reg == X86_REG_RBP;
}

bool FunctionFramepointerValidator::IsStackPointer(uint16_t reg) {
  return reg == X86_REG_SP || reg == X86_REG_ESP || reg == X86_REG_RSP;
}

bool FunctionFramepointerValidator::ValidatePrologue() {
  cs_regs regs_read, regs_write;
  uint8_t read_count, write_count;

  // check the first instruction: it must be "push ebp" or "enter"
  if (cs_regs_access(handle_, &instructions_[0], regs_read, &read_count,
                     regs_write, &write_count) == 0) {
    if (instructions_[0].id == X86_INS_ENTER) {
      return true;
    }

    if (instructions_[0].id != X86_INS_PUSH || read_count != 2 ||
        !IsStackPointer(regs_read[0]) || !IsBasePointer(regs_read[1])) {
      return false;
    }
  }

  // check the second instruction: it must be "mov ebp, esp"
  if (cs_regs_access(handle_, &instructions_[1], regs_read, &read_count,
                     regs_write, &write_count) == 0) {
    if (!IsMovInstruction(instructions_[1]) || read_count != 1 ||
        !IsStackPointer(regs_read[0]) || write_count != 1 ||
        !IsBasePointer(regs_write[0])) {
      return false;
    }
  }

  return true;
}

// We will actually only check, whether there is any correct epilogue.
// It might be the case, that there are multiple function returns and that not
// all are correct. However, we would not expect a compiler to produce this,
// and for hand written assembly, we accept wrong unwinding results.
//
// When functions are tail call optimized, the callee might not have a "ret"
// after the epilogue. In this case we just assume that a "jump" after the
// epilogue is the return to the caller.
// TODO(kuebler): Better handling for tail call optimization
bool FunctionFramepointerValidator::ValidateEpilogue() {
  // check for a "leave" "ret" sequence
  for (size_t i = 0; i < instructions_count_ - 1; i++) {
    if (instructions_[i].id == X86_INS_LEAVE &&
        IsRetOrJumpInstruction(instructions_[i + 1])) {
      return true;
    }
  }

  // check for "mov esp, ebp" "pop ebp" "ret" sequence
  for (size_t i = 0; i < instructions_count_ - 2; i++) {
    if (!IsMovInstruction(instructions_[i])) {
      continue;
    }
    if (instructions_[i + 1].id != X86_INS_POP) {
      continue;
    }
    if (!IsRetOrJumpInstruction(instructions_[i + 2])) {
      continue;
    }

    cs_regs regs_read, regs_write;
    uint8_t read_count, write_count;

    // Check first instruction to be "mov esp, ebp".
    // 1. try to get which registers has been accessed.
    if (cs_regs_access(handle_, &instructions_[i], regs_read, &read_count,
                       regs_write, &write_count) != 0) {
      continue;
    }
    // 2. Is there one read (base pointer) and one write (stack pointer).
    if (read_count != 1 || write_count != 1) {
      continue;
    }
    // 3. Check if the registers are actually the correct ones.
    if (!IsBasePointer(regs_read[0]) || !IsStackPointer(regs_write[0])) {
      continue;
    }

    // Check second instruction to be "pop ebp".
    // 1. try to get which registers has been accessed.
    if (cs_regs_access(handle_, &instructions_[i + 1], regs_read, &read_count,
                       regs_write, &write_count) != 0) {
      continue;
    }
    // 2. Is there one read (stack pointer) and two writes
    // (stack pointer and base pointer).
    if (read_count != 1 || write_count != 2) {
      continue;
    }

    // 3. Check if the registers are actually the correct ones.
    if (IsStackPointer(regs_read[0]) && IsStackPointer(regs_write[0]) &&
        IsBasePointer(regs_write[1])) {
      return true;
    }
  }

  return false;
}

bool FunctionFramepointerValidator::ValidateFramePointers() {
  return ValidatePrologue() && ValidateEpilogue();
}

bool FunctionFramepointerValidator::IsLeafFunction() {
  for (size_t i = 0; i < instructions_count_; i++) {
    if (IsCallInstruction(instructions_[i])) {
      return false;
    }
  }
  return true;
}

bool FunctionFramepointerValidator::Validate() {
  if (instructions_count_ == 0) {
    std::cout << "ERROR: Failed to disassemble given code!\n";
    return false;
  }
  bool validated =
      IsLeafFunction() || (instructions_count_ >= 4 && ValidateFramePointers());

  return validated;
}