// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeReport/Disassembler.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <stdint.h>

#include <algorithm>
#include <utility>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"

namespace orbit_code_report {
namespace {
bool IsCallInstruction(const cs_insn& instruction) {
  for (uint8_t i = 0; i < instruction.detail->groups_count; i++) {
    if (instruction.detail->groups[i] == X86_GRP_CALL) {
      return true;
    }
  }
  return false;
}
}  // namespace
void Disassembler::Disassemble(orbit_client_data::ProcessData& process,
                               orbit_client_data::ModuleManager& module_manager,
                               const void* machine_code, size_t size, uint64_t address,
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
  if (err != CS_ERR_OK) {
    AddLine(absl::StrFormat("Failed on cs_open() with error returned: %u", err));
    return;
  }

  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  count = cs_disasm(handle, static_cast<const uint8_t*>(machine_code), size, address, 0, &insn);

  if (count != 0u) {
    size_t j;

    for (j = 0; j < count; j++) {
      cs_insn* current_instruction = &insn[j];
      if (IsCallInstruction(insn[j])) {
        const orbit_client_data::FunctionInfo* callee = nullptr;

        int immediate_operands_count = cs_op_count(handle, current_instruction, X86_OP_IMM);
        if (immediate_operands_count == 1) {
          int index = cs_op_index(handle, current_instruction, X86_OP_IMM, 1);
          uint64_t immediate_operand = current_instruction->detail->x86.operands[index].imm;
          callee = orbit_client_data::FindFunctionByAddress(process, module_manager,
                                                            immediate_operand, /*is_exact=*/false);
        }

        if (callee != nullptr) {
          AddLine(absl::StrFormat("0x%llx:\t%-12s %s (%s)", current_instruction->address,
                                  current_instruction->mnemonic, current_instruction->op_str,
                                  callee->pretty_name()),
                  current_instruction->address);
          continue;
        }
        AddLine(absl::StrFormat("0x%llx:\t%-12s %s (%s)", current_instruction->address,
                                current_instruction->mnemonic, current_instruction->op_str,
                                orbit_client_data::kUnknownFunctionOrModuleName),
                insn[j].address);
        continue;
      }

      AddLine(absl::StrFormat("0x%llx:\t%-12s %s", current_instruction->address,
                              current_instruction->mnemonic, current_instruction->op_str),
              current_instruction->address);
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

std::optional<size_t> Disassembler::GetLineAtAddress(uint64_t address) const {
  const auto it = address_to_line_.find(address);
  if (it == address_to_line_.end()) return std::nullopt;
  return it->second;
}

void Disassembler::AddLine(std::string line, std::optional<uint64_t> address) {
  if (address.has_value()) address_to_line_[*address] = line_to_address_.size();
  line_to_address_.push_back(address.value_or(0ul));

  // Remove any new line character.
  line = absl::StrReplaceAll(line, {{"\n", ""}});
  result_ += absl::StrFormat("%s\n", line);
}
}  // namespace orbit_code_report
