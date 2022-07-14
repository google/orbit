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

#include <string>

#include <memory>
#include <mutex>
#include <unordered_map>

#include <unwindstack/Arch.h>
#include <unwindstack/Error.h>
#include <unwindstack/Object.h>
#include <unwindstack/PeCoffInterface.h>
#include <unwindstack/SharedString.h>

namespace unwindstack {

class MapInfo;
class Regs;
class Memory;
struct ErrorData;

// Inspects the first bytes of the file / memory to check if this is potentially a PE/COFF file.
// Since we don't read the full file and don't validate if this is really a proper PE/COFF file,
// this should only be considered a hint.
bool IsPotentiallyPeCoffFile(Memory* memory);
bool IsPotentiallyPeCoffFile(const std::string& filename);

class PeCoff : public Object {
 public:
  explicit PeCoff(Memory* memory) : memory_(memory) {}
  ~PeCoff() override = default;

  bool Init() override;
  bool valid() override { return valid_; }
  void Invalidate() override;

  int64_t GetLoadBias() override { return load_bias_; };
  bool GetTextRange(uint64_t* addr, uint64_t* size) override;
  uint64_t GetTextOffsetInFile() const;
  uint64_t GetSizeOfImage() const;

  std::string GetBuildID() override;
  std::string GetSoname() override;
  bool GetFunctionName(uint64_t, SharedString*, uint64_t*) override;
  bool GetGlobalVariableOffset(const std::string&, uint64_t*) override;

  ArchEnum arch() override { return arch_; }

  uint64_t GetRelPc(uint64_t pc, MapInfo* map_info) override;

  bool StepIfSignalHandler(uint64_t rel_pc, Regs* regs, Memory* process_memory) override;
  bool Step(uint64_t rel_pc, uint64_t pc_adjustment, Regs* regs, Memory* process_memory,
            bool* finished, bool* is_signal_frame) override;

  Memory* memory() override { return memory_.get(); }

  void GetLastError(ErrorData* data) override;
  ErrorCode GetLastErrorCode() override;
  uint64_t GetLastErrorAddress() override;

 protected:
  PeCoffInterface* CreateInterfaceFromMemory(Memory* memory);

  bool valid_ = false;
  int64_t load_bias_ = 0;
  std::unique_ptr<PeCoffInterface> interface_;
  std::unique_ptr<Memory> memory_;

  ArchEnum arch_;

  // Protect calls that can modify internal state of the interface object.
  std::mutex lock_;
};

}  // namespace unwindstack
