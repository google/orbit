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

#include <string.h>
#include <memory>
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

  UnwindCode() { memset(this, 0, sizeof(UnwindCode)); }

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
  uint8_t version_and_flags = 0;
  uint8_t prolog_size = 0;
  uint8_t num_codes = 0;
  // Low 4 bits frame register, second 4 bits frame register offset.
  uint8_t frame_register_and_offset = 0;
  std::vector<UnwindCode> unwind_codes;

  uint64_t exception_handler_address = 0;
  RuntimeFunction chained_info{0, 0, 0};

  uint8_t GetVersion() const { return version_and_flags & 0x07; }
  uint8_t GetFlags() const { return (version_and_flags >> 3) & 0x1f; }
  bool HasChainedInfo() const { return GetFlags() & 0x04; }
  uint8_t GetFrameRegister() const { return frame_register_and_offset & 0x0f; }
  uint8_t GetFrameOffset() const { return (frame_register_and_offset >> 4) & 0x0f; }
};

class PeCoffUnwindInfos {
 public:
  virtual ~PeCoffUnwindInfos() = default;

  virtual bool GetUnwindInfo(uint64_t unwind_info_rva, UnwindInfo** unwind_info) = 0;
  ErrorData GetLastError() const { return last_error_; }

 protected:
  ErrorData last_error_{ERROR_NONE, 0};
};

std::unique_ptr<PeCoffUnwindInfos> CreatePeCoffUnwindInfos(Memory* object_file_memory,
                                                           std::vector<Section> sections);

}  // namespace unwindstack
