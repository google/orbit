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

#include "PeCoffRuntimeFunctions.h"

namespace unwindstack {

bool PeCoffRuntimeFunctions::Init(uint64_t pdata_begin, uint64_t pdata_end) {
  // Since pdata_begin and pdata_end are read from the file, we can't CHECK here.
  if (pdata_end < pdata_begin) {
    return false;
  }
  const uint64_t pdata_size = pdata_end - pdata_begin;
  if (pdata_size % sizeof(RuntimeFunction) != 0) {
    return false;
  }

  runtime_functions_.resize(pdata_size / sizeof(RuntimeFunction));
  pe_coff_memory_->set_cur_offset(pdata_begin);
  return pe_coff_memory_->GetFully(static_cast<void*>(&runtime_functions_[0]), pdata_size);
}

// Binary search on the vector of runtime functions, which are guaranteed to be sorted per
// Windows PE/COFF specification
// (https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64). Using a binary search here
// for performance makes a huge difference (as opposed to a linear search) as this function is
// called on every unwinding step and there can be a lot of entries in the vector.
bool PeCoffRuntimeFunctions::FindRuntimeFunction(uint64_t pc_rva,
                                                 RuntimeFunction* runtime_function) const {
  size_t begin = 0;
  size_t end = runtime_functions_.size();
  while (begin < end) {
    size_t current = (begin + end) / 2;
    const RuntimeFunction& function = runtime_functions_[current];
    if (pc_rva >= function.start_address && pc_rva < function.end_address) {
      *runtime_function = function;
      return true;
    }
    if (function.start_address > pc_rva) {
      end = current;
    }
    if (function.end_address <= pc_rva) {
      begin = current + 1;
    }
  }
  return false;
}

}  // namespace unwindstack