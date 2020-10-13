// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Disassembler.h"

#include <absl/strings/str_format.h>
#include <capstone/capstone.h>
#include <capstone/platform.h>

namespace orbit_gl {

ErrorMessageOr<DisassembledCode> Disassemble(CodeSegmentView code, std::string title) {
  const cs_arch arch = CS_ARCH_X86;
  const cs_mode mode = code.architecture == Architecture::kX86_64 ? CS_MODE_64 : CS_MODE_32;

  csh handle = 0;
  const auto err = cs_open(arch, mode, &handle);
  if (err) {
    return outcome::failure(absl::StrFormat("Failed on cs_open() with error returned: %u", err));
  }

  auto machine_code = static_cast<const uint8_t*>(code.machine_code);
  auto machine_code_size = code.size;
  auto address = code.starting_address;

  std::string disassembly;
  std::vector<uint64_t> line_to_address;

  auto insn = cs_malloc(handle);
  try {
    while (cs_disasm_iter(handle, &machine_code, &machine_code_size, &address, insn)) {
      disassembly.append(
          absl::StrFormat("0x%llx:\t%-12s %s\n", insn->address, insn->mnemonic, insn->op_str));
      line_to_address.emplace_back(insn->address);
    }
  } catch (...) {
    cs_free(insn, 1);
    cs_close(&handle);
    throw;
  }

  cs_free(insn, 1);
  cs_close(&handle);

  return DisassembledCode{code.architecture, std::move(title), std::move(disassembly),
                          std::move(line_to_address)};
}

}  // namespace orbit_gl