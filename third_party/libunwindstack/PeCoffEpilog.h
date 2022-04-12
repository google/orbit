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

#ifndef _LIBUNWINDSTACK_PE_COFF_EPILOG_H
#define _LIBUNWINDSTACK_PE_COFF_EPILOG_H

#include <memory>

#include <unwindstack/Error.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>

namespace unwindstack {

// Helper class for epilog detection and handling used for X86_64 unwinding for PE/COFF modules.
class PeCoffEpilog {
 public:
  virtual ~PeCoffEpilog() {}

  // Needs to be called before one can use DetectAndHandleEpilog.
  virtual bool Init() = 0;

  // Detects if the instructions from 'current_offset_from_start_of_function' onwards represent
  // a function epilog. Returns true if an epilog was detected. The registers are updated to reflect
  // the actions from executing the epilog (which effectively unwinds the current callframe).
  // Returns false if no epilog was found *or* if an error occured. In the latter case, the error
  // can be retrieved using GetLastError() and registers 'regs' are not updated.
  virtual bool DetectAndHandleEpilog(uint64_t function_start_address, uint64_t function_end_address,
                                     uint64_t current_offset_from_start_of_function,
                                     Memory* process_memory, Regs* regs) = 0;

  ErrorData GetLastError() const { return last_error_; }

 protected:
  ErrorData last_error_{ERROR_NONE, 0};
};

std::unique_ptr<PeCoffEpilog> CreatePeCoffEpilog(Memory* object_file_memory,
                                                 uint64_t text_section_vmaddr,
                                                 uint64_t text_section_offset);

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_PE_COFF_EPILOG_H