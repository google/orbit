/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <cstddef>
#include <cstring>

#include <unwindstack/Memory.h>

namespace unwindstack {

// MemoryLocalUnsafe is a prototype class to compare the performance of MemoryLocal::Read
// to an "unsafe" read that assumes the local memory address provided is valid (i.e. memory is
// not corrupted and address range lies within the stack).
class MemoryLocalUnsafe : public Memory {
 public:
  MemoryLocalUnsafe() = default;
  virtual ~MemoryLocalUnsafe() = default;

  size_t Read(uint64_t addr, void* dst, size_t size) override {
    memcpy(dst, reinterpret_cast<void*>(addr), size);
    return size;
  }
};

}  // namespace unwindstack
