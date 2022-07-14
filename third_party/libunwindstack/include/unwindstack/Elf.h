/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <stddef.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include <unwindstack/Arch.h>
#include <unwindstack/ElfInterface.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Object.h>
#include <unwindstack/SharedString.h>

#if !defined(EM_AARCH64)
#define EM_AARCH64 183
#endif

namespace unwindstack {

// Forward declaration.
class MapInfo;
class Regs;

class Elf : public Object {
 public:
  Elf(Memory* memory) : memory_(memory) {}
  ~Elf() override = default;

  bool Init() override;
  bool valid() override { return valid_; }
  void Invalidate() override;

  int64_t GetLoadBias() override { return load_bias_; }
  bool GetTextRange(uint64_t* addr, uint64_t* size) override;
  std::string GetBuildID() override;

  std::string GetSoname() override;
  bool GetFunctionName(uint64_t addr, SharedString* name, uint64_t* func_offset) override;
  bool GetGlobalVariableOffset(const std::string& name, uint64_t* memory_offset) override;

  ArchEnum arch() override { return arch_; }

  uint64_t GetRelPc(uint64_t pc, MapInfo* map_info) override;

  bool StepIfSignalHandler(uint64_t rel_pc, Regs* regs, Memory* process_memory) override;
  bool Step(uint64_t rel_pc, uint64_t pc_adjustment, Regs* regs, Memory* process_memory,
            bool* finished, bool* is_signal_frame) override;

  Memory* memory() override { return memory_.get(); }

  void GetLastError(ErrorData* data) override;
  ErrorCode GetLastErrorCode() override;
  uint64_t GetLastErrorAddress() override;

  void InitGnuDebugdata();

  ElfInterface* CreateInterfaceFromMemory(Memory* memory);

  bool IsValidPc(uint64_t pc);

  uint32_t machine_type() { return machine_type_; }

  uint8_t class_type() { return class_type_; }

  ElfInterface* interface() { return interface_.get(); }

  ElfInterface* gnu_debugdata_interface() { return gnu_debugdata_interface_.get(); }

  static bool IsValidElf(Memory* memory);

  static bool GetInfo(Memory* memory, uint64_t* size);

  static int64_t GetLoadBias(Memory* memory);

  static std::string GetBuildID(Memory* memory);

 protected:
  bool valid_ = false;
  int64_t load_bias_ = 0;
  std::unique_ptr<ElfInterface> interface_;
  std::unique_ptr<Memory> memory_;
  uint32_t machine_type_;
  uint8_t class_type_;
  ArchEnum arch_;
  // Protect calls that can modify internal state of the interface object.
  std::mutex lock_;

  std::unique_ptr<Memory> gnu_debugdata_memory_;
  std::unique_ptr<ElfInterface> gnu_debugdata_interface_;
};

}  // namespace unwindstack
