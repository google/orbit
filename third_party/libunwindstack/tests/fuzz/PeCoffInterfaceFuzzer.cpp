
/*
 * Copyright 2021 The Android Open Source Project
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

#include <inttypes.h>
#include <cstddef>

#include <unwindstack/Memory.h>
#include <unwindstack/PeCoffInterface.h>

// The most basic fuzzer for PE/COFF parsing. Generates really poor coverage as
// it is not PE/COFF file structure aware.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::shared_ptr<unwindstack::Memory> memory =
      unwindstack::Memory::CreateOfflineMemory(data, 0, size);
  unwindstack::PeCoffInterface<uint64_t> pe_coff_interface(memory.get());
  return 0;
}