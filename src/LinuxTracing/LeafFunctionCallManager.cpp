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

// Let's unwind the stack from $rsp to $rbp using libunwindstack. If $rbp points to the current
// frame, this will only include the locals and not the return address, so libunwindstack will
// only be able to unwind one frame (the instruction pointer). If $rpb points to the previous frame,
// as the leaf function does not contain frame pointers, the region from $rsp to $rbp will contain
// the complete innermost frame, plus the locals of the caller's frame (but not the return address).
// Therefore, libunwindstack will be able to compute two frames, where the outer one is the missing
// caller and needs to be patched in.
// As libunwindstack will try to unwind even further, unwinding errors will always be reported.
// We need to fall back to plausibility checks to detect actual unwinding errors (such as whether
// the frames are executable).
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
  ORBIT_CHECK(event_data->GetCallchainSize() > 2);

  const uint64_t rbp = event_data->GetRegisters()[PERF_REG_X86_BP];
  const uint64_t rsp = event_data->GetRegisters()[PERF_REG_X86_SP];
  const uint64_t ip = event_data->GetRegisters()[PERF_REG_X86_IP];

  if (rbp < rsp) {
    return Callstack::kFramePointerUnwindingError;
  }

  // If the frame pointer register is set correctly at the current instruction, there is no need
  // to patch the callstack and we can early out.
  std::optional<bool> has_frame_pointer_or_error =
      unwinder->HasFramePointerSet(ip, current_maps->Get());

  // If retrieving the debug information already failed here, we don't need to try unwinding.
  if (!has_frame_pointer_or_error.has_value()) {
    return Callstack::kStackTopDwarfUnwindingError;
  }

  if (*has_frame_pointer_or_error) {
    return Callstack::kComplete;
  }

  uint64_t stack_size = rbp - rsp;

  // TODO(b/228445173): If we are in a leaf function and the stack dump contains the return address
  //  we should be able to patch the stack even if the stack dump is not from $rsp to $rbp.
  if (stack_size > stack_dump_size_) {
    return Callstack::kStackTopForDwarfUnwindingTooSmall;
  }

  const LibunwindstackResult& libunwindstack_result =
      unwinder->Unwind(event_data->pid, current_maps->Get(), event_data->GetRegisters(),
                       event_data->GetStackData(), stack_size, true);
  const std::vector<unwindstack::FrameData>& libunwindstack_callstack =
      libunwindstack_result.frames();

  if (libunwindstack_callstack.empty()) {
    ORBIT_ERROR(
        "Discarding sample as DWARF-based unwinding resulted in empty callstack (error: %s)",
        LibunwindstackUnwinder::LibunwindstackErrorString(libunwindstack_result.error_code()));
    return Callstack::kStackTopDwarfUnwindingError;
  }

  // In case of an intact frame pointer, the region from $rsp to $rbp will only include the locals
  // of the current frame, and NOT the return address. Thus, unwinding will only report one frame
  // (the instruction pointer) and we know that the original callchain is already correct.
  if (libunwindstack_callstack.size() == 1) {
    return Callstack::kComplete;
  }

  // If unwinding results in more than two frames, $rbp was also not set correctly by the caller,
  // thus frame pointers are not all in non-leaf functions, and our assumptions do not hold.
  if (libunwindstack_callstack.size() > 2) {
    return Callstack::kFramePointerUnwindingError;
  }

  const std::vector<uint64_t> original_callchain = event_data->CopyOfIpsAsVector();
  ORBIT_CHECK(original_callchain.size() > 2);

  std::vector<uint64_t> result;
  result.reserve(original_callchain.size() + 1);
  for (size_t i = 0; i < 2; ++i) {
    result.push_back(original_callchain[i]);
  }

  ORBIT_CHECK(libunwindstack_callstack.size() == 2);
  uint64_t libunwindstack_leaf_caller_pc = libunwindstack_callstack[1].pc;

  // If the caller is not executable, we have an unwinding error.
  unwindstack::MapInfo* caller_map_info = current_maps->Find(libunwindstack_leaf_caller_pc);
  if (caller_map_info == nullptr || (caller_map_info->flags() & PROT_EXEC) == 0) {
    return Callstack::kStackTopDwarfUnwindingError;
  }

  // perf_event_open's callstack always contains the return address. Libunwindstack has already
  // decreased the address by one. To not mix them, increase the address again.
  result.push_back(libunwindstack_leaf_caller_pc + 1);

  for (size_t i = 2; i < original_callchain.size(); ++i) {
    result.push_back(original_callchain[i]);
  }

  event_data->SetIps(result);

  return Callstack::kComplete;
}

}  // namespace orbit_linux_tracing