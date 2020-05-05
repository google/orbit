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

bool FunctionFramepointerValidator::isCallInstruction(
    const cs_insn& instruction) {
  for (uint8_t i = 0; i < instruction.detail->groups_count; i++) {
    if (instruction.detail->groups[i] == X86_GRP_CALL) return true;
  }
  return false;
}

bool FunctionFramepointerValidator::isMovInstruction(
    const cs_insn& instruction) {
  return instruction.id == X86_INS_MOV || instruction.id == X86_INS_MOVQ;
}

bool FunctionFramepointerValidator::isBasePointer(uint16_t reg) {
  return reg == X86_REG_BP || reg == X86_REG_EBP || reg == X86_REG_RBP;
}

bool FunctionFramepointerValidator::isStackPointer(uint16_t reg) {
  return reg == X86_REG_SP || reg == X86_REG_ESP || reg == X86_REG_RSP;
}

// we will actually only check, whether there is a correct epilogue
bool FunctionFramepointerValidator::validateEpilogue() {
  bool found_epilogue = false;
  for (size_t i = 0; i < instructions_count_ - 1; i++) {
    if (isMovInstruction(instructions_[i]) &&
        instructions_[i + 1].id == X86_INS_POP) {
      cs_regs regs_read_first, regs_write_first, regs_read_second,
          regs_write_second;
      uint8_t read_count_first, write_count_first, read_count_second,
          write_count_second;
      // check for "mov esp, ebp; pop ebp"
      if (  // first instruction
          cs_regs_access(handle_, &instructions_[i], regs_read_first,
                         &read_count_first, regs_write_first,
                         &write_count_first) == 0 &&
          read_count_first == 1 && isBasePointer(regs_read_first[0]) &&
          write_count_first == 1 && isStackPointer(regs_write_first[0]) &&
          // second instructions
          cs_regs_access(handle_, &instructions_[i + 1], regs_read_second,
                         &read_count_second, regs_write_second,
                         &write_count_second) == 0 &&
          read_count_second == 1 && isStackPointer(regs_read_second[0]) &&
          write_count_second == 2 && isStackPointer(regs_write_second[0]) &&
          isBasePointer(regs_write_second[1])) {
        found_epilogue = true;
      }
    }
  }
  return found_epilogue;
}

bool FunctionFramepointerValidator::validatePrologue() {
  cs_regs regs_read, regs_write;
  uint8_t read_count, write_count;

  // check the first instruction: it must be "push ebp"
  if (cs_regs_access(handle_, &instructions_[0], regs_read, &read_count,
                     regs_write, &write_count) == 0) {
    if (instructions_[0].id != X86_INS_PUSH || read_count != 2 ||
        !isStackPointer(regs_read[0]) || !isBasePointer(regs_read[1])) {
      return false;
    }
  }

  // check the second instruction: it must be "mov ebp, esp"
  if (cs_regs_access(handle_, &instructions_[1], regs_read, &read_count,
                     regs_write, &write_count) == 0) {
    if (!isMovInstruction(instructions_[1]) || read_count != 1 ||
        !isStackPointer(regs_read[0]) || write_count != 1 ||
        !isBasePointer(regs_write[0])) {
      return false;
    }
  }

  return true;
}

bool FunctionFramepointerValidator::validateFramePointers() {
  return validateEpilogue() && validatePrologue();
}

bool FunctionFramepointerValidator::isLeafFunction() {
  for (size_t i = 0; i < instructions_count_; i++) {
    if (isCallInstruction(instructions_[i])) {
      return false;
    }
  }
  return true;
}

bool FunctionFramepointerValidator::validate() {
  if (instructions_count_ > 0) {
    bool validated = isLeafFunction() ||
                     (instructions_count_ >= 4 && validateFramePointers());

    if (!validated) {
      std::cout << "ERROR: Validation failed for the following method:\n";
      for (size_t j = 0; j < instructions_count_; j++) {
        std::cout << "\t" << instructions_[j].mnemonic;
        std::cout << "\t" << instructions_[j].op_str << "\n";
      }
    }

    return validated;
  } else {
    std::cout << "ERROR: Failed to disassemble given code!\n";
    return false;
  }
}