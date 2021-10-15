/*
 * Copyright (C) 2019 The Android Open Source Project
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

#define _GNU_SOURCE 1
#include <stdint.h>
#include <string.h>

#include <string>

#if defined(__BIONIC__)

#include <gtest/gtest.h>

#include <unwindstack/DwarfSection.h>
#include <unwindstack/Elf.h>
#include <unwindstack/ElfInterface.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsGetLocal.h>
#include <unwindstack/Unwinder.h>

// This test is specific to bionic to verify that __libc_init is
// properly setting the return address to undefined so that the
// unwind properly terminates.

namespace unwindstack {

static std::string DumpFrames(const UnwinderFromPid& unwinder) {
  std::string unwind;
  for (size_t i = 0; i < unwinder.NumFrames(); i++) {
    unwind += unwinder.FormatFrame(i) + '\n';
  }
  return unwind;
}

static DwarfLocationEnum GetReturnAddressLocation(uint64_t rel_pc, DwarfSection* section) {
  if (section == nullptr) {
    return DWARF_LOCATION_INVALID;
  }

  const DwarfFde* fde = section->GetFdeFromPc(rel_pc);
  if (fde == nullptr || fde->cie == nullptr) {
    return DWARF_LOCATION_INVALID;
  }
  DwarfLocations regs;
  if (!section->GetCfaLocationInfo(rel_pc, fde, &regs, ARCH_UNKNOWN)) {
    return DWARF_LOCATION_INVALID;
  }

  auto reg_entry = regs.find(fde->cie->return_address_register);
  if (reg_entry == regs.end()) {
    return DWARF_LOCATION_INVALID;
  }
  return reg_entry->second.type;
}

static void VerifyReturnAddress(const FrameData& frame) {
  // Now go and find information about the register data and verify that the relative pc results in
  // an undefined register.
  Elf elf(Memory::CreateFileMemory(frame.map_name, 0).release());
  ASSERT_TRUE(elf.Init()) << "Failed to init elf object from " << frame.map_name.c_str();
  ASSERT_TRUE(elf.valid()) << "Elf " << frame.map_name.c_str() << " is not valid.";
  ElfInterface* interface = elf.interface();

  // Only check the eh_frame and the debug_frame since the undefined register
  // is set using a cfi directive.
  // Check debug_frame first, then eh_frame since debug_frame always
  // contains the most specific data.
  DwarfLocationEnum location = GetReturnAddressLocation(frame.rel_pc, interface->debug_frame());
  if (location == DWARF_LOCATION_UNDEFINED) {
    return;
  }

  location = GetReturnAddressLocation(frame.rel_pc, interface->eh_frame());
  ASSERT_EQ(DWARF_LOCATION_UNDEFINED, location);
}

// This test assumes that it starts from the main thread, and that the
// libc.so on device will include symbols so that function names can
// be resolved.
TEST(VerifyBionicTermination, local_terminate) {
  std::unique_ptr<Regs> regs(Regs::CreateFromLocal());

  UnwinderFromPid unwinder(512, getpid());
  unwinder.SetRegs(regs.get());

  RegsGetLocal(regs.get());
  unwinder.Unwind();
  ASSERT_LT(0U, unwinder.NumFrames());

  SCOPED_TRACE(DumpFrames(unwinder));

  // Look for the frame that includes __libc_init, there should only
  // be one and it should be the last.
  bool found = false;
  const std::vector<FrameData>& frames = unwinder.frames();
  for (size_t i = 0; i < unwinder.NumFrames(); i++) {
    const FrameData& frame = frames[i];
    if (frame.function_name == "__libc_init" && !frame.map_name.empty() &&
        std::string("libc.so") == basename(frame.map_name.c_str())) {
      ASSERT_EQ(unwinder.NumFrames(), i + 1) << "__libc_init is not last frame.";
      ASSERT_NO_FATAL_FAILURE(VerifyReturnAddress(frame));
      found = true;
    }
  }
  ASSERT_TRUE(found) << "Unable to find libc.so:__libc_init frame\n";
}

}  // namespace unwindstack

#endif
