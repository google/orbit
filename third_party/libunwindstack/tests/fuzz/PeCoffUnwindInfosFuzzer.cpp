/*
 * Copyright 2022 The Android Open Source Project
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
#include <memory>

#include <unwindstack/Memory.h>
#include <unwindstack/PeCoffInterface.h>
#include "PeCoffUnwindInfos.h"

namespace {
void FuzzPeCoffUnwindInfos(const uint8_t* data, size_t size) {
  std::shared_ptr<unwindstack::Memory> memory =
      unwindstack::Memory::CreateOfflineMemory(data, 0, size);
  std::unique_ptr<unwindstack::PeCoffUnwindInfos> pe_coff_unwind_infos(
      CreatePeCoffUnwindInfos(memory.get()));
  unwindstack::UnwindInfo info;
  // Try all possible offsets to increase coverage. This will also test the parser
  // running over the end of the memory.
  for (size_t offset = 0; offset < size; ++offset) {
    pe_coff_unwind_infos->GetUnwindInfo(offset, &info);
  }
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FuzzPeCoffUnwindInfos(data, size);
  return 0;
}
