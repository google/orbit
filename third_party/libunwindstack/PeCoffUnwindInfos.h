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

#ifndef _LIBUNWINDSTACK_PE_COFF_UNWIND_INFOS_H
#define _LIBUNWINDSTACK_PE_COFF_UNWIND_INFOS_H

#include <unordered_map>
#include <vector>

#include "PeCoffRuntimeFunctions.h"
#include "unwindstack/Error.h"
#include "unwindstack/PeCoffInterface.h"

namespace unwindstack {

union UnwindCode {
  struct {
    uint8_t code_offset;
    uint8_t unwind_op_and_op_info;
  } code_and_op;
  uint16_t frame_offset;

  uint8_t GetUnwindOp() const { return code_and_op.unwind_op_and_op_info & 0x0f; }
  uint8_t GetOpInfo() const { return (code_and_op.unwind_op_and_op_info >> 4) & 0x0f; }
};

// Per specification, the size of each unwind code in the file is 2 bytes and we use
// sizeof(UnwindCode) later when reading data from the file.
static_assert(sizeof(UnwindCode) == 2);

// Data as parsed from the UNWIND_INFO struct in a PE/COFF file, with convenience methods to access
// data that is encoded as bit subsets of bytes.
// https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-160#struct-unwind_info
struct UnwindInfo {
  // Low 3 bits are the version, other 5 bits are the flags.
  uint8_t version_and_flags;
  uint8_t prolog_size;
  uint8_t num_codes;
  // Low 4 bits frame register, second 4 bits frame register offset.
  uint8_t frame_register_and_offset;
  std::vector<UnwindCode> unwind_codes;

  uint64_t exception_handler_address;
  RuntimeFunction chained_info;

  uint8_t GetVersion() const { return version_and_flags & 0x07; }
  uint8_t GetFlags() const { return (version_and_flags >> 3) & 0x1f; }
  uint8_t GetFrameRegister() const { return frame_register_and_offset & 0x0f; }
  uint8_t GetFrameOffset() const { return (frame_register_and_offset >> 4) & 0x0f; }
};

class PeCoffUnwindInfos {
 public:
  explicit PeCoffUnwindInfos(PeCoffMemory* pe_coff_memory) : pe_coff_memory_(pe_coff_memory) {}

  // The value 'unwind_info_file_offset' is the file offset derived from the unwind info
  // offset in the RUNTIME_FUNCTION struct, which is a relative virtual address (RVA). Computing the
  // file offset from the unwind info RVA requires knowledge about the sections of the file, which
  // the class using 'PeCoffUnwindInfos' must use to pass in the right value here.
  bool GetUnwindInfo(uint64_t unwind_info_file_offset, UnwindInfo* unwind_info);
  ErrorData GetLastError() const { return last_error_; }

 private:
  bool ParseUnwindInfoAtOffset(uint64_t file_offset, UnwindInfo* unwind_info);

  PeCoffMemory* pe_coff_memory_;
  ErrorData last_error_{ERROR_NONE, 0};

  // This is a cache of unwind infos.
  std::unordered_map<uint64_t, UnwindInfo> unwind_info_offset_to_unwind_info_;
};

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_PE_COFF_UNWIND_INFOS_H