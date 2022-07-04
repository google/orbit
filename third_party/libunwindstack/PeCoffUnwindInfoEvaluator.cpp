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

#include "PeCoffUnwindInfoEvaluator.h"

#include <unwindstack/Log.h>
#include <limits>
#include "unwindstack/MachineX86_64.h"
#include "unwindstack/Regs.h"

namespace unwindstack {

class PeCoffUnwindInfoEvaluatorImpl : public PeCoffUnwindInfoEvaluator {
 public:
  PeCoffUnwindInfoEvaluatorImpl() {}

  // This function should only be called when we know that we are not in the epilog of the function.
  // If one attempts to unwind using this when one is actually on an instruction in the epilog, the
  // results will most likely be wrong.
  // The function will skip unwind codes as needed based on 'current_code_offset', e.g. when we are
  // in the middle of the prolog and not all instructions in the prolog have been executed yet.
  bool Eval(Memory* process_memory, Regs* regs, const UnwindInfo& unwind_info,
            PeCoffUnwindInfos* unwind_infos, uint64_t current_code_offset) override;
};

std::unique_ptr<PeCoffUnwindInfoEvaluator> CreatePeCoffUnwindInfoEvaluator() {
  return std::make_unique<PeCoffUnwindInfoEvaluatorImpl>();
}

// The order of registers in PE/COFF unwind information is different from the libunwindstack
// register order, so we have to map them to the right values. See
// https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-160#operation-info
static uint16_t MapToUnwindstackRegister(uint8_t op_info_register) {
  static constexpr std::array<uint16_t, 16> kMachineToUnwindstackRegister = {
      X86_64_REG_RAX, X86_64_REG_RCX, X86_64_REG_RDX, X86_64_REG_RBX,
      X86_64_REG_RSP, X86_64_REG_RBP, X86_64_REG_RSI, X86_64_REG_RDI,
      X86_64_REG_R8,  X86_64_REG_R9,  X86_64_REG_R10, X86_64_REG_R11,
      X86_64_REG_R12, X86_64_REG_R13, X86_64_REG_R14, X86_64_REG_R15};

  if (op_info_register >= kMachineToUnwindstackRegister.size()) {
    return X86_64_REG_LAST;
  }

  return kMachineToUnwindstackRegister[op_info_register];
}

bool PeCoffUnwindInfoEvaluatorImpl::Eval(Memory* process_memory, Regs* regs,
                                         const UnwindInfo& unwind_info,
                                         PeCoffUnwindInfos* unwind_infos,
                                         uint64_t current_code_offset) {
  // Data is parsed from the object file, so we have to assume that it may be inconsistent.
  if (unwind_info.num_codes != unwind_info.unwind_codes.size()) {
    last_error_.code = ERROR_INVALID_COFF;
    return false;
  }

  RegsImpl<uint64_t>* cur_regs = static_cast<RegsImpl<uint64_t>*>(regs);

  for (int op_idx = 0; op_idx < unwind_info.num_codes;) {
    const UnwindCode unwind_code = unwind_info.unwind_codes[op_idx];
    switch (unwind_code.GetUnwindOp()) {
      case UWOP_PUSH_NONVOL: {
        if (unwind_code.code_and_op.code_offset > current_code_offset) {
          op_idx += 1;
          continue;
        }
        uint64_t register_value;
        if (!process_memory->Read64(cur_regs->sp(), &register_value)) {
          last_error_.code = ERROR_MEMORY_INVALID;
          last_error_.address = cur_regs->sp();
          return false;
        }
        cur_regs->set_sp(cur_regs->sp() + sizeof(uint64_t));

        const uint8_t op_info = unwind_code.GetOpInfo();
        const uint16_t reg = MapToUnwindstackRegister(op_info);
        (*cur_regs)[reg] = register_value;

        op_idx++;
        break;
      }
      case UWOP_ALLOC_LARGE: {
        const uint8_t op_info = unwind_code.GetOpInfo();
        uint32_t allocation_size = 0;

        if (op_info == 0) {
          if (unwind_code.code_and_op.code_offset > current_code_offset) {
            // Must be the total number of indices we have to increase by.
            op_idx += 2;
            continue;
          }
          if (op_idx + 1 >= unwind_info.num_codes) {
            last_error_.code = ERROR_INVALID_COFF;
            return false;
          }
          const UnwindCode offset = unwind_info.unwind_codes[op_idx + 1];
          allocation_size = 8 * static_cast<uint32_t>(offset.frame_offset);
          op_idx += 2;
        } else if (op_info == 1) {
          if (unwind_code.code_and_op.code_offset > current_code_offset) {
            // Must be the total number of indices we have to increase by.
            op_idx += 3;
            continue;
          }
          if (op_idx + 2 >= unwind_info.num_codes) {
            last_error_.code = ERROR_INVALID_COFF;
            return false;
          }
          const UnwindCode offset1 = unwind_info.unwind_codes[op_idx + 1];
          const UnwindCode offset2 = unwind_info.unwind_codes[op_idx + 2];

          allocation_size = static_cast<uint32_t>(offset1.frame_offset) +
                            (static_cast<uint32_t>(offset2.frame_offset) << 16);
          op_idx += 3;
        } else {
          last_error_.code = ERROR_INVALID_COFF;
          return false;
        }

        cur_regs->set_sp(cur_regs->sp() + allocation_size);
        break;
      }
      case UWOP_ALLOC_SMALL: {
        if (unwind_code.code_and_op.code_offset > current_code_offset) {
          op_idx += 1;
          continue;
        }
        const uint8_t op_info = unwind_code.GetOpInfo();
        const uint32_t allocation_size = static_cast<uint32_t>(op_info) * 8 + 8;

        cur_regs->set_sp(cur_regs->sp() + allocation_size);

        op_idx += 1;
        break;
      }
      case UWOP_SET_FPREG: {
        if (unwind_code.code_and_op.code_offset > current_code_offset) {
          op_idx += 1;
          continue;
        }
        const uint8_t frame_register = unwind_info.GetFrameRegister();
        const uint16_t reg = MapToUnwindstackRegister(frame_register);
        const uint64_t frame_offset = 16 * static_cast<uint64_t>(unwind_info.GetFrameOffset());

        if (frame_offset > (*cur_regs)[reg]) {
          last_error_.code = ERROR_INVALID_COFF;
          return false;
        }
        uint64_t new_sp_value = (*cur_regs)[reg] - frame_offset;
        cur_regs->set_sp(new_sp_value);

        op_idx += 1;
        break;
      }
      case UWOP_SAVE_NONVOL: {
        if (unwind_code.code_and_op.code_offset > current_code_offset) {
          op_idx += 2;
          continue;
        }

        if (op_idx + 1 >= unwind_info.num_codes) {
          last_error_.code = ERROR_INVALID_COFF;
          return false;
        }

        const UnwindCode offset = unwind_info.unwind_codes[op_idx + 1];
        const uint32_t save_offset = 8 * static_cast<uint32_t>(offset.frame_offset);

        const uint8_t op_info = unwind_code.GetOpInfo();
        const uint16_t reg = MapToUnwindstackRegister(op_info);

        uint64_t value;
        if (!process_memory->Read64(cur_regs->sp() + save_offset, &value)) {
          last_error_.code = ERROR_MEMORY_INVALID;
          last_error_.address = cur_regs->sp() + save_offset;
          return false;
        }

        (*cur_regs)[reg] = value;

        op_idx += 2;
        break;
      }
      case UWOP_SAVE_NONVOL_FAR: {
        if (unwind_code.code_and_op.code_offset > current_code_offset) {
          op_idx += 3;
          continue;
        }
        if (op_idx + 2 >= unwind_info.num_codes) {
          last_error_.code = ERROR_INVALID_COFF;
          return false;
        }
        const UnwindCode offset1 = unwind_info.unwind_codes[op_idx + 1];
        const UnwindCode offset2 = unwind_info.unwind_codes[op_idx + 2];

        const uint32_t save_offset = static_cast<uint32_t>(offset1.frame_offset) +
                                     (static_cast<uint32_t>(offset2.frame_offset) << 16);

        const uint8_t op_info = unwind_code.GetOpInfo();
        const uint16_t reg = MapToUnwindstackRegister(op_info);

        uint64_t value;
        if (!process_memory->Read64(cur_regs->sp() + save_offset, &value)) {
          last_error_.code = ERROR_MEMORY_INVALID;
          last_error_.address = cur_regs->sp() + save_offset;
          return false;
        }

        (*cur_regs)[reg] = value;

        op_idx += 3;
        break;
      }
      case UWOP_EPILOG: {
        // This is an undocumented opcode from the rare and undocumented version 2 of UNWIND_INFO.
        // We know that it takes two slots, but the meaning of the operation info and of the second
        // slot is not certain. The purpose seems to be to describe the location of the epilogs of
        // the function, which would speed up unwinding by removing the need for epilog detection.
        if (unwind_info.GetVersion() != 2) {
          last_error_.code = ERROR_INVALID_COFF;
          return false;
        }
        // As this is rare and undocumented, just do nothing for now and rely on epilog detection as
        // in version 1.
        op_idx += 2;
        break;
      }
      case UWOP_SAVE_XMM128: {
        // We do not actually have to save the XMM registers here and in the UWOP_SAVE_XMM128_FAR
        // case, we just have to skip the unwind codes. XMM registers are not read by other unwind
        // operations, so they do not influence the actual frame unwinding. Setting them here has
        // only informational purposes if we want to display the contents of the registers (e.g. in
        // a debugger).
        op_idx += 2;
        break;
      }
      case UWOP_SAVE_XMM128_FAR: {
        // See comment for UWOP_SAVE_XMM128.
        op_idx += 3;
        break;
      }
      case UWOP_PUSH_MACHFRAME: {
        // TODO: Support all op codes.
        last_error_.code = ERROR_UNSUPPORTED;
        last_error_.address = 0;
        return false;
      }
      default: {
        last_error_.code = ERROR_INVALID_COFF;
        last_error_.address = 0;
        return false;
      }
    }
  }

  if (unwind_info.HasChainedInfo()) {
    UnwindInfo* chained_unwind_info;
    if (!unwind_infos->GetUnwindInfo(unwind_info.chained_info.unwind_info_offset,
                                     &chained_unwind_info)) {
      return false;
    }

    // We have to chain all unwind operations that are in the chained info, so we pass the max
    // uint64_t value as code offset.
    return Eval(process_memory, regs, *chained_unwind_info, unwind_infos,
                std::numeric_limits<uint64_t>::max());
  }

  return true;
}

}  // namespace unwindstack