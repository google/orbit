// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LeafFunctionCallManager.h"

#include <sys/mman.h>

#include <vector>

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
    CallchainSamplePerfEvent* event, LibunwindstackMaps* current_maps,
    LibunwindstackUnwinder* unwinder) {
  CHECK(event != nullptr);
  CHECK(current_maps != nullptr);
  CHECK(unwinder != nullptr);
  CHECK(event->GetCallchainSize() > 2);

  uint64_t rbp = event->GetRegisters()[PERF_REG_X86_BP];
  uint64_t rsp = event->GetRegisters()[PERF_REG_X86_SP];

  if (rbp < rsp) {
    return Callstack::kFramePointerUnwindingError;
  }

  uint64_t stack_size = rbp - rsp;
  if (stack_size > stack_dump_size_) {
    return Callstack::kStackTopForDwarfUnwindingTooSmall;
  }

  const LibunwindstackResult& libunwindstack_result =
      unwinder->Unwind(event->GetPid(), current_maps->Get(), event->GetRegisters(),
                       event->GetStackData(), stack_size, true);
  const std::vector<unwindstack::FrameData>& libunwindstack_callstack =
      libunwindstack_result.frames();

  if (libunwindstack_callstack.empty()) {
    ERROR("Discarding sample as DWARF-based unwinding resulted in empty callstack (error: %s)",
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

  const std::vector<uint64_t> original_callchain = event->ips;
  CHECK(original_callchain.size() > 2);

  std::vector<uint64_t> result;
  result.reserve(original_callchain.size() + 1);
  for (size_t i = 0; i < 2; ++i) {
    result.push_back(original_callchain[i]);
  }

  CHECK(libunwindstack_callstack.size() == 2);
  uint64_t libunwindstack_leaf_caller_pc = libunwindstack_callstack[1].pc;

  // If the caller is not executable, we have an unwinding error.
  unwindstack::MapInfo* map_info = current_maps->Find(libunwindstack_leaf_caller_pc);
  if (map_info == nullptr || (map_info->flags & PROT_EXEC) == 0) {
    return Callstack::kStackTopDwarfUnwindingError;
  }

  // perf_event_open's callstack always contains the return address. Libunwindstack has already
  // decreased the address by one. To not mix them, increase the address again.
  result.push_back(libunwindstack_leaf_caller_pc + 1);

  for (size_t i = 2; i < original_callchain.size(); ++i) {
    result.push_back(original_callchain[i]);
  }

  event->ring_buffer_record.nr = result.size();
  event->ips = std::move(result);

  return Callstack::kComplete;
}

}  // namespace orbit_linux_tracing