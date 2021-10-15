/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <unwindstack/JitDebug.h>

#include <unwindstack/Elf.h>

#include "GlobalDebugImpl.h"
#include "MemoryBuffer.h"

namespace unwindstack {

template <>
bool GlobalDebugInterface<Elf>::Load(Maps*, std::shared_ptr<Memory>& memory, uint64_t addr,
                                     uint64_t size, /*out*/ std::shared_ptr<Elf>& elf) {
  std::unique_ptr<MemoryBuffer> copy(new MemoryBuffer());
  if (!copy->Resize(size) || !memory->ReadFully(addr, copy->GetPtr(0), size)) {
    return false;
  }
  elf.reset(new Elf(copy.release()));
  return elf->Init() && elf->valid();
}

std::unique_ptr<JitDebug> CreateJitDebug(ArchEnum arch, std::shared_ptr<Memory>& memory,
                                         std::vector<std::string> search_libs) {
  return CreateGlobalDebugImpl<Elf>(arch, memory, search_libs, "__jit_debug_descriptor");
}

}  // namespace unwindstack
