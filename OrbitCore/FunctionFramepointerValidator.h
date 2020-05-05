// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_FUNCTION_FRAMEPOINTER_VALIDATOR_H_
#define ORBIT_CORE_FUNCTION_FRAMEPOINTER_VALIDATOR_H_

#include <capstone/capstone.h>

/*
 * Provide utilities to check whether a function was compiled with
 * "-fno-omit-frame-pointer (-momit-leaf-frame-pointer)". The latter one is
 * optional.
 * The validator checks the functions Prologue and also whether an Epilogue
 * exists.
 */
class FunctionFramepointerValidator {
 public:
  FunctionFramepointerValidator(csh handle, const uint8_t* code,
                                size_t code_size);
  virtual ~FunctionFramepointerValidator();

  // FunctionFramepointerValidator is neither copyable nor movable.
  FunctionFramepointerValidator(const FunctionFramepointerValidator&) = delete;
  FunctionFramepointerValidator& operator=(
      const FunctionFramepointerValidator&) = delete;

  bool validate();

 private:
  bool isCallInstruction(const cs_insn& instruction);
  bool isMovInstruction(const cs_insn& instruction);
  bool isBasePointer(uint16_t reg);
  bool isStackPointer(uint16_t reg);
  bool isLeafFunction();
  bool validateEpilogue();
  bool validatePrologue();
  bool validateFramePointers();

  cs_insn* instructions_;
  size_t instructions_count_;
  csh handle_;
};

#endif  // ORBIT_CORE_FUNCTION_FRAMEPOINTER_VALIDATOR_H_
