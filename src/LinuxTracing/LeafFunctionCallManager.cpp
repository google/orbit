// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LeafFunctionCallManager.h"

#include <absl/base/casts.h>
#include <asm/perf_regs.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Unwinder.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

using orbit_grpc_protos::Callstack;

// Let's unwind one frame using libunwindstack. With that unwinding step, the registers will get
// updated and we can detect if $rbp was modified.
// (1) If $rbp did not change: We are in a leaf function, which has not modified $rbp. The leaf's
//     caller is missing in the callchain and needs to be patched in. The updated $rip (pc) from
//     the unwinding step contains the leaf's caller.
// (2) If $rbp was modified, this can either be:
//     (a) We are in a non-leaf function and the callchain is already correct.
//     (b) We are in a leaf function that modified $rbp. The complete callchain is broken and should
//         be reported as unwinding error.
// As libunwindstack does not report us the canonical frame address (CFA) from an unwinding step, we
// cannot differentiate between (2a) and (2b) reliably. However, we do perform the following
// validity checks (for the reasoning remember that the stack grows downwards):
// (I)   If the CFA is computed using $rbp + 16, we know the $rbp was correct, i.e. case (2a)
// (II)  If $rbp is below $rsp, $rbp is not a frame pointer, i.e. case (2b)
// (III) If $rbp moves up the stack after unwinding, the sampled $rbp is not a frame pointer (2b)
//
// Note that we cannot simply set libunwindstack to unwind always two frames and compare the outer
// frame with the respective one in the callchain carried by the perf_event_open event, as in case
// of uprobes overriding the return addresses, both addresses would be identical even if the actual
// addresses (after uprobe patching) are not.
// More (internal) documentation on this in: go/stadia-orbit-leaf-frame-pointer
Callstack::CallstackType LeafFunctionCallManager::PatchCallerOfLeafFunction(
    const CallchainSamplePerfEventData* event_data, LibunwindstackMaps* current_maps,
    LibunwindstackUnwinder* unwinder) {
  ORBIT_CHECK(event_data != nullptr);
  ORBIT_CHECK(current_maps != nullptr);
  ORBIT_CHECK(unwinder != nullptr);

  const uint64_t rbp = event_data->GetRegisters()[PERF_REG_X86_BP];
  const uint64_t rsp = event_data->GetRegisters()[PERF_REG_X86_SP];
  const uint64_t rip = event_data->GetRegisters()[PERF_REG_X86_IP];

  if (rbp < rsp) {
    return Callstack::kFramePointerUnwindingError;
  }

  std::optional<bool> has_frame_pointer_or_error =
      unwinder->HasFramePointerSet(rip, event_data->pid, current_maps->Get());

  // If retrieving the debug information already failed here, we don't need to try unwinding.
  if (!has_frame_pointer_or_error.has_value()) {
    return Callstack::kStackTopDwarfUnwindingError;
  }

  // If the frame pointer register is set correctly at the current instruction, there is no need
  // to patch the callstack and we can early out.
  if (*has_frame_pointer_or_error) {
    return Callstack::kComplete;
  }

  // Perform one unwinding step. We will only need the memory from $rbp + 16 to $rsp (ensure to
  // include the previous frame pointer and the return address) for unwinding. If $rbp does not
  // change from unwinding, we need to patch in the pc after unwinding.
  const uint64_t stack_size = rbp - rsp + 16;
  const LibunwindstackResult& libunwindstack_result = unwinder->Unwind(
      event_data->pid, current_maps->Get(), event_data->GetRegisters(), event_data->GetStackData(),
      std::min<uint64_t>(stack_size, stack_dump_size_), true, /*max_frames=*/1);

  // If unwinding a single frame yields a success, we are in the outer-most frame, i.e. we don't
  // have a caller to patch in.
  if (libunwindstack_result.IsSuccess()) {
    return Callstack::kComplete;
  }

  unwindstack::RegsX86_64 new_regs = libunwindstack_result.regs();

  // If both pc and $rsp do not change during unwinding, there was an unwinding error.
  if ((new_regs.pc() == rip && new_regs.sp() == rsp) || libunwindstack_result.frames().empty()) {
    // If the error was because the stack sample was too small, the user can act and increase the
    // stack size. So we report that case separately.
    if (stack_size > stack_dump_size_) {
      return Callstack::kStackTopForDwarfUnwindingTooSmall;
    }
    return Callstack::kStackTopDwarfUnwindingError;
  }

  uint64_t new_rbp = new_regs[unwindstack::X86_64_REG_RBP];
  // $rbp changed during unwinding (case (2)), i.e. either it was a valid frame pointer and thus the
  // callchain is already correct, or it was modified as general purpose register (unwinding error).
  if (new_rbp != rbp) {
    // If the $rbp after unwinding is below the sampled $rbp, the sampled $rbp could not be a valid
    // frame pointer (remember the stack grows downwards).
    // Note that, in addition to this check, we also check if the complete callchain is in
    // executable code in the UprobesUnwindingVisitor.
    if (new_rbp < rbp) {
      return Callstack::kFramePointerUnwindingError;
    }
    return Callstack::kComplete;
  }

  // $rbp did non change during unwinding, i.e. we are in a leaf function. We need to patch in the
  // missing caller, which is the updated pc from unwinding.
  const std::vector<uint64_t> original_callchain = event_data->CopyOfIpsAsVector();
  ORBIT_CHECK(original_callchain.size() >= 2);

  std::vector<uint64_t> result;
  result.reserve(original_callchain.size() + 1);
  for (size_t i = 0; i < 2; ++i) {
    result.push_back(original_callchain[i]);
  }

  uint64_t libunwindstack_leaf_caller_pc = new_regs.pc();

  // If the caller is not executable, we have an unwinding error.
  unwindstack::MapInfo* caller_map_info = current_maps->Find(libunwindstack_leaf_caller_pc);
  if (caller_map_info == nullptr || (caller_map_info->flags() & PROT_EXEC) == 0) {
    // As above, if the error was because the stack sample was too small, the user can act and
    // increase the stack size. So we report that case separately.
    if (stack_size > stack_dump_size_) {
      return Callstack::kStackTopForDwarfUnwindingTooSmall;
    }
    return Callstack::kStackTopDwarfUnwindingError;
  }

  result.push_back(libunwindstack_leaf_caller_pc);

  for (size_t i = 2; i < original_callchain.size(); ++i) {
    result.push_back(original_callchain[i]);
  }

  event_data->SetIps(result);

  return Callstack::kComplete;
}

}  // namespace orbit_linux_tracing