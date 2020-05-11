// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FUNCTION_FRAME_POINTER_VALIDATOR_H_
#define ORBIT_CORE_FUNCTION_FRAME_POINTER_VALIDATOR_H_

#include <capstone/capstone.h>

// Provide utilities to check whether a function was compiled with
// "-fno-omit-frame-pointer (-momit-leaf-frame-pointer)". The latter one is
// optional.
// The validator checks the functions Prologue and also whether an Epilogue
// exists.
class FunctionFramePointerValidator {
 public:
  FunctionFramePointerValidator(csh handle, const uint8_t* code,
                                size_t code_size);
  virtual ~FunctionFramePointerValidator();

  // FunctionFramePointerValidator is neither copyable nor movable.
  FunctionFramePointerValidator(const FunctionFramePointerValidator&) = delete;
  FunctionFramePointerValidator& operator=(
      const FunctionFramePointerValidator&) = delete;
  FunctionFramePointerValidator(FunctionFramePointerValidator&&) = delete;
  FunctionFramePointerValidator& operator=(FunctionFramePointerValidator&&) =
      delete;

  bool Validate();

 private:
  static bool IsCallInstruction(const cs_insn& instruction);
  static bool IsRetOrJumpInstruction(const cs_insn& instruction);
  static bool IsMovInstruction(const cs_insn& instruction);
  static bool IsBasePointer(uint16_t reg);
  static bool IsStackPointer(uint16_t reg);
  bool IsLeafFunction();
  bool ValidatePrologue();
  bool ValidateEpilogue();
  bool ValidateFramePointers();

  cs_insn* instructions_;
  size_t instructions_count_;
  csh handle_;
};

#endif  // ORBIT_CORE_FUNCTION_FRAME_POINTER_VALIDATOR_H_
