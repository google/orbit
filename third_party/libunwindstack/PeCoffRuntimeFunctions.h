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
#include <vector>

#include "unwindstack/Error.h"
#include "unwindstack/PeCoffInterface.h"

namespace unwindstack {

// Data as parsed from the RUNTIME_FUNCTION array.
// https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-160#struct-runtime_function
struct RuntimeFunction {
  uint32_t start_address;
  uint32_t end_address;
  uint32_t unwind_info_offset;
};

// The RUNTIME_FUNCTION struct, and thus PeCoffRuntimeFunctions, is only used on x86_64.
class PeCoffRuntimeFunctions {
 public:
  virtual ~PeCoffRuntimeFunctions() {}
  virtual bool Init(uint64_t pdata_begin, uint64_t pdata_end) = 0;
  virtual bool FindRuntimeFunction(uint64_t pc_rva, RuntimeFunction* runtime_function) const = 0;

  ErrorData GetLastError() const { return last_error_; }

 protected:
  ErrorData last_error_{ERROR_NONE, 0};
};

std::unique_ptr<PeCoffRuntimeFunctions> CreatePeCoffRuntimeFunctions(Memory* object_file_memory);

}  // namespace unwindstack
