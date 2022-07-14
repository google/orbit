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

#pragma once

#include <stdint.h>

#include <deque>
#include <string>
#include <unordered_map>

#include <unwindstack/Elf.h>
#include <unwindstack/ElfInterface.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/SharedString.h>

#include "ElfInterfaceArm.h"

namespace unwindstack {

struct StepData {
  StepData(uint64_t pc, uint64_t sp, bool finished) : pc(pc), sp(sp), finished(finished) {}
  uint64_t pc;
  uint64_t sp;
  bool finished;
};

struct FunctionData {
  FunctionData(std::string name, uint64_t offset) : name(name), offset(offset) {}

  std::string name;
  uint64_t offset;
};

class ElfFake : public Elf {
 public:
  ElfFake(Memory* memory) : Elf(memory) { valid_ = true; }
  virtual ~ElfFake() = default;

  void FakeSetValid(bool valid) { valid_ = valid; }

  void FakeSetLoadBias(uint64_t load_bias) { load_bias_ = load_bias; }

  void FakeSetArch(ArchEnum arch) { arch_ = arch; }

  void FakeSetInterface(ElfInterface* interface) { interface_.reset(interface); }
  void FakeSetGnuDebugdataInterface(ElfInterface* interface) {
    gnu_debugdata_interface_.reset(interface);
  }
};

class ElfInterfaceFake : public ElfInterface {
 public:
  ElfInterfaceFake(Memory* memory) : ElfInterface(memory) {}
  virtual ~ElfInterfaceFake() = default;

  bool Init(int64_t*) override { return false; }
  void InitHeaders() override {}
  std::string GetSoname() override { return fake_soname_; }

  bool GetFunctionName(uint64_t, SharedString*, uint64_t*) override;
  bool GetGlobalVariable(const std::string&, uint64_t*) override;
  std::string GetBuildID() override { return fake_build_id_; }

  bool Step(uint64_t, Regs*, Memory*, bool*, bool*) override;

  void FakeSetGlobalVariable(const std::string& global, uint64_t offset) {
    globals_[global] = offset;
  }

  void FakeSetBuildID(std::string& build_id) { fake_build_id_ = build_id; }
  void FakeSetBuildID(const char* build_id) { fake_build_id_ = build_id; }

  void FakeSetSoname(const char* soname) { fake_soname_ = soname; }

  static void FakePushFunctionData(const FunctionData data) { functions_.push_back(data); }
  static void FakePushStepData(const StepData data) { steps_.push_back(data); }

  static void FakeClear() {
    functions_.clear();
    steps_.clear();
  }

  void FakeSetErrorCode(ErrorCode code) { last_error_.code = code; }

  void FakeSetErrorAddress(uint64_t address) { last_error_.address = address; }

  void FakeSetDataOffset(uint64_t offset) { data_offset_ = offset; }
  void FakeSetDataVaddrStart(uint64_t vaddr) { data_vaddr_start_ = vaddr; }
  void FakeSetDataVaddrEnd(uint64_t vaddr) { data_vaddr_end_ = vaddr; }

  void FakeSetDynamicOffset(uint64_t offset) { dynamic_offset_ = offset; }
  void FakeSetDynamicVaddrStart(uint64_t vaddr) { dynamic_vaddr_start_ = vaddr; }
  void FakeSetDynamicVaddrEnd(uint64_t vaddr) { dynamic_vaddr_end_ = vaddr; }

  void FakeSetGnuDebugdataOffset(uint64_t offset) { gnu_debugdata_offset_ = offset; }
  void FakeSetGnuDebugdataSize(uint64_t size) { gnu_debugdata_size_ = size; }

 private:
  std::unordered_map<std::string, uint64_t> globals_;
  std::string fake_build_id_;
  std::string fake_soname_;

  static std::deque<FunctionData> functions_;
  static std::deque<StepData> steps_;
};

class ElfInterface32Fake : public ElfInterface32 {
 public:
  ElfInterface32Fake(Memory* memory) : ElfInterface32(memory) {}
  virtual ~ElfInterface32Fake() = default;

  void FakeSetEhFrameOffset(uint64_t offset) { eh_frame_offset_ = offset; }
  void FakeSetEhFrameSize(uint64_t size) { eh_frame_size_ = size; }
  void FakeSetDebugFrameOffset(uint64_t offset) { debug_frame_offset_ = offset; }
  void FakeSetDebugFrameSize(uint64_t size) { debug_frame_size_ = size; }
};

class ElfInterface64Fake : public ElfInterface64 {
 public:
  ElfInterface64Fake(Memory* memory) : ElfInterface64(memory) {}
  virtual ~ElfInterface64Fake() = default;

  void FakeSetEhFrameOffset(uint64_t offset) { eh_frame_offset_ = offset; }
  void FakeSetEhFrameSize(uint64_t size) { eh_frame_size_ = size; }
  void FakeSetDebugFrameOffset(uint64_t offset) { debug_frame_offset_ = offset; }
  void FakeSetDebugFrameSize(uint64_t size) { debug_frame_size_ = size; }
};

class ElfInterfaceArmFake : public ElfInterfaceArm {
 public:
  ElfInterfaceArmFake(Memory* memory) : ElfInterfaceArm(memory) {}
  virtual ~ElfInterfaceArmFake() = default;

  void FakeSetStartOffset(uint64_t offset) { start_offset_ = offset; }
  void FakeSetTotalEntries(size_t entries) { total_entries_ = entries; }
};

}  // namespace unwindstack
