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

#ifndef _LIBUNWINDSTACK_PE_COFF_UNWIND_INFO_UNWINDER_X86_64_H
#define _LIBUNWINDSTACK_PE_COFF_UNWIND_INFO_UNWINDER_X86_64_H

#include <memory>

#include "PeCoffUnwindInfos.h"
#include "unwindstack/PeCoffInterface.h"

namespace unwindstack {

// Unwind operation codes as specified on:
// https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-160#unwind-operation-code
enum UnwindOpCode : uint8_t {
  UWOP_PUSH_NONVOL = 0,
  UWOP_ALLOC_LARGE = 1,
  UWOP_ALLOC_SMALL = 2,
  UWOP_SET_FPREG = 3,
  UWOP_SAVE_NONVOL = 4,
  UWOP_SAVE_NONVOL_FAR = 5,
  // There are no codes 6 and 7
  UWOP_SAVE_XMM128 = 8,
  UWOP_SAVE_XMM128_FAR = 9,
  UWOP_PUSH_MACHFRAME = 10
};

class PeCoffUnwindInfoUnwinderX86_64 {
 public:
  explicit PeCoffUnwindInfoUnwinderX86_64(Memory* object_file_memory)
      : unwind_infos_(CreatePeCoffUnwindInfos(object_file_memory)) {}

  // This function should only be called when we know that we are not in the epilog of the function.
  // If one attempts to unwind using this when one is actually on an instruction in the epilog, the
  // results will most likely be wrong.
  // The function will skip unwind codes as needed based on 'current_code_offset', e.g. when we are
  // in the middle of the prolog and not all instructions in the prolog have been executed yet.
  bool Eval(Memory* process_memory, Regs* regs, const UnwindInfo& unwind_info,
            uint64_t current_code_offset, uint64_t* frame_pointer, bool* frame_pointer_used);

  ErrorData GetLastError() const { return last_error_; }

 private:
  ErrorData last_error_{ERROR_NONE, 0};
  std::unique_ptr<PeCoffUnwindInfos> unwind_infos_;
};

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_PE_COFF_UNWIND_INFO_UNWINDER_X86_64_H