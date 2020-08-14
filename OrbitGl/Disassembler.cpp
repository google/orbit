// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Disassembler.h"

#include <absl/strings/str_replace.h>
#include <capstone/capstone.h>
#include <capstone/platform.h>

void Disassembler::Disassemble(const void* machine_code, size_t size, uint64_t address,
                               bool is_64bit) {
  csh handle = 0;
  cs_arch arch = CS_ARCH_X86;
  cs_insn* insn = nullptr;
  size_t count = 0;
  cs_err err;
  cs_mode mode = is_64bit ? CS_MODE_64 : CS_MODE_32;

  AddLine(absl::StrFormat("Platform: %s",
                          is_64bit ? "X86 64 (Intel syntax)" : "X86 32 (Intel syntax)"));
  err = cs_open(arch, mode, &handle);
  if (err) {
    AddLine(absl::StrFormat("Failed on cs_open() with error returned: %u", err));
    return;
  }

  count = cs_disasm(handle, static_cast<const uint8_t*>(machine_code), size, address, 0, &insn);

  if (count) {
    size_t j;

    for (j = 0; j < count; j++) {
      AddLine(
          absl::StrFormat("0x%llx:\t%-12s %s", insn[j].address, insn[j].mnemonic, insn[j].op_str),
          insn[j].address);
    }

    // Print out the next offset, after the last instruction.
    AddLine(absl::StrFormat("0x%llx:", insn[j - 1].address + insn[j - 1].size));

    // Free memory allocated by cs_disasm().
    cs_free(insn, count);
  } else {
    AddLine("****************");
    AddLine("ERROR: Failed to disasm given code!");
  }

  AddLine("");
  cs_close(&handle);
}

uint64_t Disassembler::GetAddressAtLine(size_t line) const {
  if (line >= line_to_address_.size()) return 0;
  return line_to_address_[line];
}

void Disassembler::AddLine(std::string line, uint64_t address) {
  // Remove any new line character.
  line = absl::StrReplaceAll(line, {{"\n", ""}});
  line_to_address_.push_back(address);
  result_ += absl::StrFormat("%s\n", line);
}
