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

#include <unwindstack/PeCoff.h>

#include <string>

#include <unwindstack/Log.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Object.h>
#include <unwindstack/PeCoffInterface.h>
#include <unwindstack/Regs.h>
#include "MemoryFileAtOffset.h"

#include "Check.h"

namespace unwindstack {

bool IsPotentiallyPeCoffFile(Memory* memory) {
  if (memory == nullptr) {
    return false;
  }
  uint16_t magic_value;
  if (!memory->Read16(0, &magic_value)) {
    return false;
  }

  // This magic value is present in the first two bytes of every PE/COFF file. Note that there are
  // additional magic bytes later in the header that could be checked as well, but since we only
  // need a hint whether the file is PE/COFF or ELF, just checking this magic value suffices.
  constexpr uint16_t kMsDosTwoPointZeroMagicValue = 0x5a4d;
  return magic_value == kMsDosTwoPointZeroMagicValue;
}

bool IsPotentiallyPeCoffFile(const std::string& filename) {
  MemoryFileAtOffset memory;
  if (!memory.Init(filename, 0)) {
    return false;
  }

  return IsPotentiallyPeCoffFile(&memory);
}

PeCoffInterface* PeCoff::CreateInterfaceFromMemory(Memory* memory) {
  int64_t unused_load_bias;

  PeCoffInterface32 interface32(memory);
  if (interface32.Init(&unused_load_bias)) {
    arch_ = ARCH_X86;
    return new PeCoffInterface32(memory);
  }

  PeCoffInterface64 interface64(memory);
  if (interface64.Init(&unused_load_bias)) {
    arch_ = ARCH_X86_64;
    return new PeCoffInterface64(memory);
  }

  return nullptr;
}

bool PeCoff::Init() {
  load_bias_ = 0;
  if (!memory_) {
    return false;
  }

  interface_.reset(CreateInterfaceFromMemory(memory_.get()));
  if (!interface_) {
    return false;
  }

  valid_ = interface_->Init(&load_bias_);

  if (!valid_) {
    interface_.reset(nullptr);
  }
  return valid_;
}

void PeCoff::Invalidate() {
  interface_.reset(nullptr);
  valid_ = false;
}

bool PeCoff::GetTextRange(uint64_t* addr, uint64_t* size) {
  if (!valid_) {
    return false;
  }

  if (interface_->GetTextRange(addr, size)) {
    *addr += load_bias_;
    return true;
  }

  return false;
}

uint64_t PeCoff::GetTextOffsetInFile() const {
  if (!valid_) {
    return 0;
  }

  return interface_->GetTextOffsetInFile();
}

uint64_t PeCoff::GetSizeOfImage() const {
  if (!valid_) {
    return 0;
  }
  return interface_->GetSizeOfImage();
}

std::string PeCoff::GetBuildID() {
  // Not implemented, don't use.
  CHECK(false);
  return "";
}

std::string PeCoff::GetSoname() {
  // Not implemented, don't use.
  CHECK(false);
  return "";
}

bool PeCoff::GetFunctionName(uint64_t, SharedString*, uint64_t*) {
  // For PE/COFF, in many cases getting the function name will require access to a separate PDB
  // file and the ability to parse that file. Alternatives would be to get the name from the export
  // directory (only for dlls and for public symbols) or from .debug_info if the file has DWARF
  // information (e.g. for Wine dlls).
  return false;
}

bool PeCoff::GetGlobalVariableOffset(const std::string&, uint64_t*) {
  // Not implemented, don't use.
  CHECK(false);
  return false;
}

uint64_t PeCoff::GetRelPc(uint64_t pc, MapInfo* map_info) {
  if (!valid_) {
    return 0;
  }
  if (map_info->object_offset() == 0 && map_info->object_rva() != 0) {
    return interface_->GetRelPcWithMapRva(pc, map_info->start(), map_info->object_rva());
  }
  return interface_->GetRelPcWithMapOffset(pc, map_info->start(), map_info->object_offset());
}

bool PeCoff::StepIfSignalHandler(uint64_t, Regs*, Memory*) {
  return false;
}

bool PeCoff::Step(uint64_t rel_pc, uint64_t pc_adjustment, Regs* regs, Memory* process_memory,
                  bool* finished, bool* is_signal_frame) {
  if (!valid_) {
    return false;
  }
  // Lock during the step which can update information in the object.
  std::lock_guard<std::mutex> guard(lock_);
  return interface_->Step(rel_pc, pc_adjustment, regs, process_memory, finished, is_signal_frame);
}

void PeCoff::GetLastError(ErrorData* data) {
  if (valid_) {
    *data = interface_->last_error();
  } else {
    data->code = ERROR_INVALID_COFF;
    data->address = 0;
  }
}

ErrorCode PeCoff::GetLastErrorCode() {
  if (valid_) {
    return interface_->LastErrorCode();
  }
  return ERROR_INVALID_COFF;
}

uint64_t PeCoff::GetLastErrorAddress() {
  if (valid_) {
    return interface_->LastErrorAddress();
  }
  return 0;
}

}  // namespace unwindstack