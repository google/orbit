/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "PeCoffEpilog.h"

#include <map>
#include <memory>
#include <unordered_set>
#include <vector>

#include <capstone/capstone.h>
#include <capstone/x86.h>

#include <unwindstack/MachineX86_64.h>
#include "Check.h"
#include "unwindstack/PeCoffInterface.h"

namespace unwindstack {

class PeCoffEpilogImpl : public PeCoffEpilog {
 public:
  PeCoffEpilogImpl(Memory* object_file_memory, std::vector<Section> sections)
      : file_memory_(object_file_memory), sections_(std::move(sections)) {}
  ~PeCoffEpilogImpl() override;

  // Needs to be called before one can use DetectAndHandleEpilog.
  bool Init() override;

  // Detects if the instructions from 'current_offset_from_start_of_function' onwards represent
  // a function epilog. Returns true if an epilog was detected. The registers are updated to reflect
  // the actions from executing the epilog (which effectively unwinds the current callframe).
  // Returns false if no epilog was found *or* if an error occured. In the latter case, the error
  // can be retrieved using GetLastError() and registers 'regs' are not updated.
  bool DetectAndHandleEpilog(uint64_t function_start_address, uint64_t function_end_address,
                             uint64_t current_offset_from_start_of_function, Memory* process_memory,
                             Regs* regs, bool* is_in_epilog) override;

 private:
  bool DetectAndHandleEpilog(const std::vector<uint8_t>& machine_code, Memory* process_memory,
                             Regs* regs, bool* is_in_epilog);

  // The validation methods below check if the instructions satisfy the requirements imposed by the
  // epilog specification, as outlined on
  // https://docs.microsoft.com/en-us/cpp/build/prolog-and-epilog?view=msvc-170
  // The corresponding handling methods must only be called after the validation was successful.
  bool ValidateLeaInstruction();
  void HandleLeaInstruction(RegsImpl<uint64_t>* registers);
  bool ValidateAddInstruction();
  void HandleAddInstruction(RegsImpl<uint64_t>* registers);
  bool ValidatePopInstruction();
  bool HandleEightByteRegisterPopInstruction(Memory* process_memory, RegsImpl<uint64_t>* registers);
  bool HandleReturnInstruction(Memory* process_memory, RegsImpl<uint64_t>* registers);
  bool ValidateJumpInstruction();
  bool HandleJumpInstruction(Memory* process_memory, RegsImpl<uint64_t>* registers);

  bool MapFromRvaToFileOffset(uint64_t rva, uint64_t* file_offset);

  Memory* file_memory_;
  std::vector<Section> sections_;

  bool capstone_initialized_ = false;
  csh capstone_handle_ = 0;
  cs_insn* capstone_instruction_ = nullptr;

  // Cache of RVAs for which we have already successfully detected that we are *not* in an epilog
  // (and hence for which we do not have to update the registers).
  // Note that the performance benefits of this cache should be re-evaluated if/when we stop using
  // capstone in favor of a completely custom detection of legal epilog instructions.
  std::unordered_set<uint64_t> addresses_not_in_epilog_;
};

std::unique_ptr<PeCoffEpilog> CreatePeCoffEpilog(Memory* object_file_memory,
                                                 std::vector<Section> sections) {
  return std::make_unique<PeCoffEpilogImpl>(object_file_memory, std::move(sections));
}

PeCoffEpilogImpl::~PeCoffEpilogImpl() {
  if (capstone_initialized_) {
    cs_free(capstone_instruction_, 1);
    cs_close(&capstone_handle_);
  }
}

bool PeCoffEpilogImpl::Init() {
  if (file_memory_ == nullptr) {
    return false;
  }
  cs_err err = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle_);
  if (err) {
    return false;
  }
  err = cs_option(capstone_handle_, CS_OPT_DETAIL, CS_OPT_ON);
  if (err) {
    cs_close(&capstone_handle_);
    return false;
  }
  capstone_instruction_ = cs_malloc(capstone_handle_);
  capstone_initialized_ = true;
  return true;
}

bool PeCoffEpilogImpl::MapFromRvaToFileOffset(uint64_t rva, uint64_t* file_offset) {
  for (const Section& section : sections_) {
    if (section.vmaddr <= rva && rva < section.vmaddr + section.vmsize) {
      *file_offset = rva - section.vmaddr + section.offset;
      return true;
    }
  }
  last_error_.code = ERROR_INVALID_COFF;
  return false;
}

static uint16_t MapCapstoneToUnwindstackRegister(x86_reg capstone_reg) {
  static const std::map<x86_reg, uint16_t> kMapToUnwindstackRegister = {
      {X86_REG_RAX, X86_64_REG_RAX}, {X86_REG_RCX, X86_64_REG_RCX}, {X86_REG_RDX, X86_64_REG_RDX},
      {X86_REG_RBX, X86_64_REG_RBX}, {X86_REG_RSP, X86_64_REG_RSP}, {X86_REG_RBP, X86_64_REG_RBP},
      {X86_REG_RSI, X86_64_REG_RSI}, {X86_REG_RDI, X86_64_REG_RDI}, {X86_REG_R8, X86_64_REG_R8},
      {X86_REG_R9, X86_64_REG_R9},   {X86_REG_R10, X86_64_REG_R10}, {X86_REG_R11, X86_64_REG_R11},
      {X86_REG_R12, X86_64_REG_R12}, {X86_REG_R13, X86_64_REG_R13}, {X86_REG_R14, X86_64_REG_R14},
      {X86_REG_R15, X86_64_REG_R15}};
  auto it = kMapToUnwindstackRegister.find(capstone_reg);
  if (it != kMapToUnwindstackRegister.end()) {
    return it->second;
  }
  return X86_64_REG_LAST;
}

bool PeCoffEpilogImpl::ValidateLeaInstruction() {
  // Note that this instruction is only legal as the first instruction if frame pointers are
  // being used.
  // TODO: Do we need to check that this frame is using a frame pointer? Can be seen in the
  // unwind info. I believe this has no impact on unwinding itself and would thus only be a
  // check that the compiler actually emitted instructions correctly. Probably not worth it to
  // check here.
  CHECK(capstone_instruction_->detail->x86.op_count == 2);
  cs_x86_op operand0 = capstone_instruction_->detail->x86.operands[0];

  // First operand is always a register for 'lea' instruction.
  CHECK(operand0.type == X86_OP_REG);

  x86_reg reg = operand0.reg;
  if (reg != X86_REG_RSP) {
    // The register that we set must be rsp, o/w we are not in the epilog.
    return false;
  }
  cs_x86_op operand1 = capstone_instruction_->detail->x86.operands[1];

  // Second operand is always a mem operand for 'lea' instructions.
  CHECK(operand1.type == X86_OP_MEM);

  // TODO: Not sure if this is really illegal.
  CHECK(operand1.mem.segment == X86_REG_INVALID);

  if (operand1.mem.index != X86_REG_INVALID) {
    // Only instructions of the form lea rsp, constant[frame_pointer_register] are legal. This
    // excludes using an index register.
    return false;
  }

  return true;
}

void PeCoffEpilogImpl::HandleLeaInstruction(RegsImpl<uint64_t>* registers) {
  cs_x86_op operand1 = capstone_instruction_->detail->x86.operands[1];
  x86_reg base_reg = operand1.mem.base;
  uint16_t unwindstack_base_reg = MapCapstoneToUnwindstackRegister(base_reg);

  uint64_t effective_address = (*registers)[unwindstack_base_reg] + operand1.mem.disp;
  registers->set_sp(effective_address);
}

bool PeCoffEpilogImpl::ValidateAddInstruction() {
  CHECK(capstone_instruction_->detail->x86.op_count == 2);
  cs_x86_op operand0 = capstone_instruction_->detail->x86.operands[0];
  cs_x86_op operand1 = capstone_instruction_->detail->x86.operands[1];
  if (operand0.type != X86_OP_REG || operand1.type != X86_OP_IMM) {
    // The 'add' instruction must be adding an immediate value to a register, o/w
    // we are not in the epilog.
    return false;
  }
  x86_reg reg = operand0.reg;
  if (reg != X86_REG_RSP) {
    // The register that we add to must be rsp, o/w we are not in the epilog.
    return false;
  }
  int64_t immediate_value = operand1.imm;
  if (immediate_value < 0) {
    // The immediate value represents the stack allocation size, so it must be non-negative.
    return false;
  }
  return true;
}

void PeCoffEpilogImpl::HandleAddInstruction(RegsImpl<uint64_t>* registers) {
  // An 'add' instruction in the epilog adds the immediate value to the stack pointer to deallocate
  // the stack frame.
  int64_t immediate_value = capstone_instruction_->detail->x86.operands[1].imm;
  registers->set_sp(registers->sp() + static_cast<uint64_t>(immediate_value));
}

bool PeCoffEpilogImpl::ValidatePopInstruction() {
  // All pop instructions have exactly one operand.
  CHECK(capstone_instruction_->detail->x86.op_count == 1);
  cs_x86_op operand = capstone_instruction_->detail->x86.operands[0];

  // Only "8-byte register pops" are allowed in an epilog.
  return operand.type == X86_OP_REG && operand.size == 8;
}

bool PeCoffEpilogImpl::HandleEightByteRegisterPopInstruction(Memory* process_memory,
                                                             RegsImpl<uint64_t>* registers) {
  CHECK(capstone_instruction_->detail->x86.op_count == 1);
  cs_x86_op operand = capstone_instruction_->detail->x86.operands[0];
  CHECK(operand.type == X86_OP_REG);
  CHECK(operand.size == 8);
  x86_reg reg = operand.reg;
  uint16_t unwindstack_reg = MapCapstoneToUnwindstackRegister(reg);

  // Handling a pop instruction means reading the value on top of the stack, then setting the
  // register operand of the instruction with the read value, and increasing the stack pointer.
  uint64_t value;
  if (!process_memory->Read64(registers->sp(), &value)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = registers->sp();
    return false;
  }

  registers->set_sp(registers->sp() + sizeof(uint64_t));
  (*registers)[unwindstack_reg] = value;

  return true;
}

bool PeCoffEpilogImpl::HandleReturnInstruction(Memory* process_memory,
                                               RegsImpl<uint64_t>* registers) {
  // Handling the return means we have to read the return address from the top of the stack, and
  // then set stack pointer and pc accordingly.
  uint64_t return_address;
  if (!process_memory->Read64(registers->sp(), &return_address)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = registers->sp();
    return false;
  }
  registers->set_sp(registers->sp() + sizeof(uint64_t));
  registers->set_pc(return_address);

  return true;
}

bool PeCoffEpilogImpl::ValidateJumpInstruction() {
  // It's not entirely clear how to distinguish between regular 'jmp' instructions and 'jmp'
  // instructions that are at the end of an epilog (e.g. due to tail call optimization).
  // There are some restrictions which 'jmp' instructions are allowed in epilogs, but this
  // doesn't solve the problem of distinguishing entirely. This means that we may identify all
  // 'jmp' instructions that satisfy this restriction as an epilog consisting of a single
  // instruction.
  // TODO: We may need to look at the unwind codes of this function to see if the epilog should
  // be non-trivial (and not just consist of a single 'jmp' instruction).

  // Only 'jmp' instructions with memory references are allowed in the epilog:
  // https://docs.microsoft.com/en-us/cpp/build/prolog-and-epilog?view=msvc-170#epilog-code
  CHECK(capstone_instruction_->detail->x86.op_count >= 1);
  cs_x86_op operand0 = capstone_instruction_->detail->x86.operands[0];
  if (operand0.type != X86_OP_MEM) {
    return false;
  }

  // Only instructions with mod = 0b00 are allowed according to
  // https://docs.microsoft.com/en-us/cpp/build/prolog-and-epilog?view=msvc-170#epilog-code
  // (The modrm byte consist of fields mod, reg, and rm, where mod is 2 bits, reg is 3 bits, and
  // rm is 3 bits.)
  if (capstone_instruction_->detail->x86.modrm & 0b11'000'000) {
    return false;
  }

  return true;
}

bool PeCoffEpilogImpl::HandleJumpInstruction(Memory* process_memory,
                                             RegsImpl<uint64_t>* registers) {
  // Seeing a 'jmp' at the end of the epilog means we are jumping into some other function that
  // will carry out prolog and at the end epilog instructions, setting up and unwinding a
  // callframe. We do not have to simulate all these steps by the function we are jumping to,
  // the return address leading back to the function that called the current function is already
  // on the top of the stack and we can just directly virtually return here. Hence, the rest of
  // the code to handle this case is exactly the same as in the 'ret' case.
  uint64_t return_address;
  if (!process_memory->Read64(registers->sp(), &return_address)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = registers->sp();
    return false;
  }
  registers->set_sp(registers->sp() + sizeof(uint64_t));
  registers->set_pc(return_address);

  return true;
}

// Disassembles the machine code passed in, and scans through the instructions one-by-one to detect
// if the machine code is an epilog according to the specification. For x86_64 on on Windows,
// epilogs must follow a specific pattern as described on:
// https://docs.microsoft.com/en-us/cpp/build/prolog-and-epilog?view=msvc-170
// Instructions are virtually executed and their effect reflected on the registers if we are indeed
// in an epilog. Since we are detecting and handling the epilog at the same time, we must make sure
// that registers are not changed if we detect later that we are indeed not in an epilog.
bool PeCoffEpilogImpl::DetectAndHandleEpilog(const std::vector<uint8_t>& machine_code,
                                             Memory* process_memory, Regs* regs,
                                             bool* is_in_epilog) {
  CHECK(process_memory != nullptr);
  // These values are all updated by capstone as we go through the machine code for disassembling.
  uint64_t current_offset = 0;
  size_t current_code_size = machine_code.size();
  const uint8_t* current_code_pointer = machine_code.data();

  bool is_first_iteration = true;

  // We need to copy registers to make sure we don't overwrite values incorrectly when after some
  // instructions we find out we are actually not in the epilog.
  CHECK(regs != nullptr);
  std::unique_ptr<Regs> cloned_regs(regs->Clone());
  auto* updated_regs = static_cast<RegsImpl<uint64_t>*>(cloned_regs.get());

  bool have_seen_ret_or_jmp = false;

  while (current_code_size > 0) {
    if (!cs_disasm_iter(capstone_handle_, &current_code_pointer, &current_code_size,
                        &current_offset, capstone_instruction_)) {
      last_error_.code = ERROR_UNSUPPORTED;
      return false;
    }

    // The instructions 'lea' and 'add' are only legal as the first instruction of the epilog, so we
    // can only see them in the first iteration of this loop if we are indeed in the epilog. In this
    // case we are actually at the start of the epilog.
    if (is_first_iteration && capstone_instruction_->id == X86_INS_LEA) {
      if (!ValidateLeaInstruction()) {
        *is_in_epilog = false;
        return true;
      }
      HandleLeaInstruction(updated_regs);
    } else if (is_first_iteration && capstone_instruction_->id == X86_INS_ADD) {
      if (!ValidateAddInstruction()) {
        *is_in_epilog = false;
        return true;
      }
      HandleAddInstruction(updated_regs);
    } else if (capstone_instruction_->id == X86_INS_POP) {
      if (!ValidatePopInstruction()) {
        *is_in_epilog = false;
        return true;
      }
      if (!HandleEightByteRegisterPopInstruction(process_memory, updated_regs)) {
        CHECK(last_error_.code != ERROR_NONE);
        return false;
      }
    } else if (capstone_instruction_->id == X86_INS_RET ||
               capstone_instruction_->id == X86_INS_RETF) {
      if (!HandleReturnInstruction(process_memory, updated_regs)) {
        CHECK(last_error_.code != ERROR_NONE);
        return false;
      }

      // This is the last instruction of the epilog.
      have_seen_ret_or_jmp = true;
      break;
    } else if (capstone_instruction_->id == X86_INS_JMP) {
      if (!ValidateJumpInstruction()) {
        *is_in_epilog = false;
        return true;
      }
      if (!HandleJumpInstruction(process_memory, updated_regs)) {
        CHECK(last_error_.code != ERROR_NONE);
        return false;
      }
      // This is the last instruction of the epilog.
      have_seen_ret_or_jmp = true;
      break;
    } else {
      *is_in_epilog = false;
      return true;
    }

    is_first_iteration = false;
  }
  if (!have_seen_ret_or_jmp) {
    *is_in_epilog = false;
    return true;
  }

  // If we get here, then we indeed were in the epilog and must update all proper registers to the
  // updated registers that followed the epilog instructions.
  auto* current_regs = static_cast<RegsImpl<uint64_t>*>(regs);
  for (uint16_t reg = 0; reg < X86_64_REG_LAST; ++reg) {
    (*current_regs)[reg] = (*updated_regs)[reg];
  }
  *is_in_epilog = true;
  return true;
}

bool PeCoffEpilogImpl::DetectAndHandleEpilog(uint64_t function_start_address,
                                             uint64_t function_end_address,
                                             uint64_t current_offset_from_start_of_function,
                                             Memory* process_memory, Regs* regs,
                                             bool* is_in_epilog) {
  last_error_ = {ERROR_NONE, 0};

  const uint64_t current_address = function_start_address + current_offset_from_start_of_function;

  if (addresses_not_in_epilog_.count(current_address) > 0) {
    *is_in_epilog = false;
    return true;
  }

  if (function_start_address + current_offset_from_start_of_function >= function_end_address) {
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  uint64_t start_offset{};
  if (!MapFromRvaToFileOffset(current_address, &start_offset)) {
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  size_t code_size = function_end_address - current_address;
  std::vector<uint8_t> code;
  code.resize(code_size);

  CHECK(file_memory_ != nullptr);
  // Note: It may be tempting to try reading the machine code from the process memory, which also
  // contains the machine code (as the process has the object file loaded). However, normally only
  // the stack portion relevant for unwinding is readily available in 'process_memory'. While other
  // memory accesses are supported, they involve stopping the target process to read out the memory.
  // Overall this is slower (seen in experiments) and directly affects the target process.
  if (!file_memory_->ReadFully(start_offset, static_cast<void*>(code.data()), code_size)) {
    last_error_.code = ERROR_MEMORY_INVALID;
    last_error_.address = start_offset;
    return false;
  }

  bool detection_succeeded = DetectAndHandleEpilog(code, process_memory, regs, is_in_epilog);
  if (detection_succeeded && !*is_in_epilog) {
    addresses_not_in_epilog_.insert(current_address);
  }
  return detection_succeeded;
}

}  // namespace unwindstack