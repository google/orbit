/*
 * Copyright 2020 The Android Open Source Project
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

#include <functional>
#include <iostream>
#include <vector>

#include <unwindstack/JitDebug.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Unwinder.h>

#include "UnwinderComponentCreator.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "utils/MemoryFake.h"

namespace unwindstack {

static constexpr int kMaxUnwindStringLen = 50;
static constexpr int kMaxUnwindStrings = 50;

void PerformUnwind(FuzzedDataProvider* data_provider, Unwinder* unwinder) {
  // 0 = don't set any values
  // 1 = set initial_map_names_to_skip
  // 2 = set map_suffixes_to_ignore
  // 3 = set both
  uint8_t set_values = data_provider->ConsumeIntegral<uint8_t>() % 4;
  if (set_values == 0) {
    unwinder->Unwind();
  } else if (set_values == 1) {
    // Only setting initial_map_names_to_skip
    std::vector<std::string> skip_names =
        GetStringList(data_provider, kMaxUnwindStringLen, kMaxUnwindStrings);

    unwinder->Unwind(&skip_names, nullptr);
  } else if (set_values == 2) {
    // Only setting map_suffixes_to_ignore
    std::vector<std::string> ignore_suffixes =
        GetStringList(data_provider, kMaxUnwindStringLen, kMaxUnwindStrings);

    unwinder->Unwind(nullptr, &ignore_suffixes);
  } else if (set_values == 3) {
    // Setting both values
    std::vector<std::string> skip_names =
        GetStringList(data_provider, kMaxUnwindStringLen, kMaxUnwindStrings);
    std::vector<std::string> ignore_suffixes =
        GetStringList(data_provider, kMaxUnwindStringLen, kMaxUnwindStrings);

    unwinder->Unwind(&skip_names, &ignore_suffixes);
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FuzzedDataProvider data_provider(data, size);

  // We need to construct an unwinder.
  // Generate the Maps:
  std::unique_ptr<Maps> maps = GetMaps(&data_provider);

  // Generate the Regs:
  uint8_t arch_val = data_provider.ConsumeIntegralInRange<uint8_t>(1, kArchCount);
  ArchEnum arch = static_cast<ArchEnum>(arch_val);
  std::unique_ptr<Regs> regs = GetRegisters(arch);

  // Generate memory:
  std::shared_ptr<Memory> memory = std::make_shared<MemoryFake>();
  PutElfFilesInMemory(reinterpret_cast<MemoryFake*>(memory.get()), &data_provider);

  size_t max_frames = data_provider.ConsumeIntegralInRange<size_t>(0, 5000);

  std::unique_ptr<JitDebug> jit_debug_ptr = CreateJitDebug(arch, memory);

  // Create instance
  Unwinder unwinder(max_frames, maps.get(), regs.get(), memory);
  unwinder.SetJitDebug(jit_debug_ptr.get());
  unwinder.SetResolveNames(data_provider.ConsumeBool());
  // Call unwind
  PerformUnwind(&data_provider, &unwinder);

  // Run some additional logic that changes after unwind
  uint64_t pc = data_provider.ConsumeIntegral<uint64_t>();
  unwinder.BuildFrameFromPcOnly(pc);
  unwinder.ConsumeFrames();
  return 0;
}
}  // namespace unwindstack
