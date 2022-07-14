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

#include <unwindstack/DwarfSection.h>
#include <unwindstack/Memory.h>

namespace unwindstack {

template <typename TypeParam>
class DwarfSectionImplFake : public DwarfSectionImpl<TypeParam> {
 public:
  DwarfSectionImplFake(Memory* memory) : DwarfSectionImpl<TypeParam>(memory) {}
  virtual ~DwarfSectionImplFake() = default;

  bool Init(uint64_t, uint64_t, int64_t) override { return false; }

  void GetFdes(std::vector<const DwarfFde*>*) override {}

  const DwarfFde* GetFdeFromPc(uint64_t) override { return nullptr; }

  uint64_t GetCieOffsetFromFde32(uint32_t) { return 0; }

  uint64_t GetCieOffsetFromFde64(uint64_t) { return 0; }

  uint64_t AdjustPcFromFde(uint64_t) override { return 0; }

  void FakeSetCachedCieLocRegs(uint64_t offset, const DwarfLocations& loc_regs) {
    this->cie_loc_regs_[offset] = loc_regs;
  }
  void FakeClearError() { this->last_error_.code = DWARF_ERROR_NONE; }
};

}  // namespace unwindstack
