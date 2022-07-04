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

#include <limits>
#include "unwindstack/MachineX86_64.h"
#include "unwindstack/Regs.h"

namespace unwindstack {

bool PeCoffUnwindInfoUnwinderX86_64::Step(uint64_t pc, uint64_t pc_adjustment, Regs* regs,
                                          Memory* process_memory, bool* finished,
                                          bool* is_signal_frame) {
  // Indicates that we are in the stack frame of a signal handler, which is never the case
  // when unwinding a PE/COFF frame.
  *is_signal_frame = false;

  uint64_t pc_rva = pc - image_base_ + pc_adjustment;

  RegsImpl<uint64_t>* cur_regs = static_cast<RegsImpl<uint64_t>*>(regs);

  RuntimeFunction function_at_pc;
  if (!runtime_functions_->FindRuntimeFunction(pc_rva, &function_at_pc)) {
    // By specification, if we cannot find a runtime function at the current PC, we are in
    // a leaf function, which for PE/COFF are precisely the functions that do not adjust any
    // of the callee-saved registers (also called non-volatile registers), including the stack
    // pointer. This implies that the stack pointer points to the return address and we can just
    // read it out.
    uint64_t return_address;
    if (!process_memory->Read64(cur_regs->sp(), &return_address)) {
      last_error_.code = ERROR_MEMORY_INVALID;
      last_error_.address = cur_regs->sp();
      return false;
    }
    cur_regs->set_pc(return_address);
    cur_regs->set_sp(cur_regs->sp() + sizeof(uint64_t));

    *finished = (regs->pc() == 0);
    return true;
  }

  UnwindInfo* unwind_info;
  if (!unwind_infos_->GetUnwindInfo(function_at_pc.unwind_info_offset, &unwind_info)) {
    last_error_ = unwind_infos_->GetLastError();
    return false;
  }

  // If we are beyond the prolog, that is, the current PC offset from the start of the function is
  // larger than the prolog size indicated in the unwind info, we need to check if we are in an
  // epilog of the function. If yes, the registers, including SP and PC, are already adjusted by
  // 'DetectAndHandleEpilog' and we can return here. If no, then we must unwind using the entire
  // sequence of the unwind codes.
  //
  // An important optimization is that we only have to detect whether we are in an epilog when
  // unwinding the innermost frame. For other frames, we know that the non-adjusted PC is on a call
  // instruction, i.e., if the adjusted PC is in an epilog, it can only be on the first instruction
  // of that epilog: when this is the case, no instruction of that epilog has been executed, so
  // unwinding by handling all the instructions of the epilog is equivalent to processing the entire
  // sequence of UNWIND_CODEs. So we simply always do the latter whether we are already at the
  // beginning of an epilog or not.
  // Conveniently, we know we are unwinding the innermost frame if and only if pc_adjustment == 0
  // (the value is 1 for all other frames).
  uint64_t current_offset_from_start = pc_rva - function_at_pc.start_address;
  if (pc_adjustment == 0 && current_offset_from_start > unwind_info->prolog_size) {
    bool is_in_epilog;
    // If 'DetectAndHandleEpilog' fails with an error, we have to return here.
    if (!epilog_->DetectAndHandleEpilog(function_at_pc.start_address, function_at_pc.end_address,
                                        current_offset_from_start, process_memory, regs,
                                        &is_in_epilog)) {
      last_error_ = epilog_->GetLastError();
      return false;
    }
    if (is_in_epilog) {
      *finished = (regs->pc() == 0);
      return true;
    }
  }

  if (!unwind_info_evaluator_->Eval(process_memory, regs, *unwind_info, unwind_infos_.get(),
                                    current_offset_from_start)) {
    last_error_ = unwind_info_evaluator_->GetLastError();
    return false;
  }

  uint64_t return_address;
  if (!process_memory->Read64(cur_regs->sp(), &return_address)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = cur_regs->sp();
    return false;
  }
  cur_regs->set_sp(cur_regs->sp() + sizeof(uint64_t));
  cur_regs->set_pc(return_address);

  *finished = (regs->pc() == 0);

  return true;
}

}  // namespace unwindstack