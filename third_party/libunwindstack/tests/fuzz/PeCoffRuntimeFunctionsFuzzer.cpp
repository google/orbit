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
#include "PeCoffRuntimeFunctions.h"

namespace {
void FuzzPeCoffRuntimeFunctions(const uint8_t* data, size_t size) {
  std::shared_ptr<unwindstack::Memory> memory =
      unwindstack::Memory::CreateOfflineMemory(data, 0, size);
  std::unique_ptr<unwindstack::PeCoffRuntimeFunctions> pe_coff_runtime_functions(
      CreatePeCoffRuntimeFunctions(memory.get()));
  pe_coff_runtime_functions->Init(0, size);
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FuzzPeCoffRuntimeFunctions(data, size);
  return 0;
}
