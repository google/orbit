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

#pragma once

#include <memory>

#include <unwindstack/Error.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include "unwindstack/PeCoffInterface.h"

namespace unwindstack {

// Helper class for epilog detection and handling used for X86_64 unwinding for PE/COFF modules.
class PeCoffEpilog {
 public:
  virtual ~PeCoffEpilog() = default;

  // Needs to be called before one can use DetectAndHandleEpilog.
  virtual bool Init() = 0;

  // Detects if the instructions from 'current_offset_from_start_of_function' onwards represent
  // a function epilog. The registers are updated to reflect the actions from executing the epilog
  // (which effectively unwinds the current callframe).
  // Returns true if no error occurred while detecting if the instructions represent an epilog, and
  // no error occurred while possibly updating the registers. In this case, the 'is_in_epilog'
  // output boolean will contain whether an epilog was detected.
  // Returns false if an error occurred while trying to detect if the instructions represent an
  // epilog, or while trying to update the registers. In this case, registers 'regs' and
  // 'is_in_epilog' are not updated, and the error can be retrieved using 'GetLastError()'.
  virtual bool DetectAndHandleEpilog(uint64_t function_start_address, uint64_t function_end_address,
                                     uint64_t current_offset_from_start_of_function,
                                     Memory* process_memory, Regs* regs, bool* is_in_epilog) = 0;

  ErrorData GetLastError() const { return last_error_; }

 protected:
  ErrorData last_error_{ERROR_NONE, 0};
};

std::unique_ptr<PeCoffEpilog> CreatePeCoffEpilog(Memory* object_file_memory,
                                                 std::vector<Section> sections);

}  // namespace unwindstack
