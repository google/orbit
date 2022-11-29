// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LeafFunctionCallManager.h"

#include <sys/mman.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

#include "LibunwindstackMultipleOfflineAndProcessMemory.h"
#include "OrbitBase/Logging.h"
#include "unwindstack/MachineX86_64.h"
#include "unwindstack/MapInfo.h"
#include "unwindstack/Regs.h"
#include "unwindstack/RegsX86_64.h"

namespace orbit_linux_tracing {
template <typename CallchainPerfEventDataT>
orbit_grpc_protos::Callstack::CallstackType LeafFunctionCallManager::PatchCallerOfLeafFunctionImpl(
    const CallchainPerfEventDataT* event_data, LibunwindstackMaps* current_maps,
    LibunwindstackUnwinder* unwinder) {
  ORBIT_CHECK(event_data != nullptr);
  ORBIT_CHECK(current_maps != nullptr);
  ORBIT_CHECK(unwinder != nullptr);

  const uint64_t rbp = event_data->GetRegisters().bp;
  const uint64_t rsp = event_data->GetRegisters().sp;
  const uint64_t rip = event_data->GetRegisters().ip;

  if (rbp < rsp) {
    return orbit_grpc_protos::Callstack::kFramePointerUnwindingError;
  }

  std::optional<bool> has_frame_pointer_or_error = unwinder->HasFramePointerSet(
      rip, event_data->GetCallstackPidOrMinusOne(), current_maps->Get());

  // If retrieving the debug information already failed here, we don't need to try unwinding.
  if (!has_frame_pointer_or_error.has_value()) {
    return orbit_grpc_protos::Callstack::kStackTopDwarfUnwindingError;
  }

  // If the frame pointer register is set correctly at the current instruction, there is no need
  // to patch the callstack and we can early out.
  if (*has_frame_pointer_or_error) {
    return orbit_grpc_protos::Callstack::kComplete;
  }

  // Perform one unwinding step. We will only need the memory from $rbp + 16 to $rsp (ensure to
  // include the previous frame pointer and the return address) for unwinding. If $rbp does not
  // change from unwinding, we need to patch in the pc after unwinding.
  const uint64_t stack_size = rbp - rsp + 16;
  StackSliceView stack_slice{event_data->GetRegisters().sp,
                             std::min<uint64_t>(stack_size, stack_dump_size_),
                             event_data->data.get()};
  std::vector<StackSliceView> stack_slices{stack_slice};
  const LibunwindstackResult& libunwindstack_result =
      unwinder->Unwind(event_data->GetCallstackPidOrMinusOne(), current_maps->Get(),
                       event_data->GetRegistersAsArray(), stack_slices, true, /*max_frames=*/1);

  // If unwinding a single frame yields a success, we are in the outer-most frame, i.e. we don't
  // have a caller to patch in.
  if (libunwindstack_result.IsSuccess()) {
    return orbit_grpc_protos::Callstack::kComplete;
  }

  unwindstack::RegsX86_64 new_regs = libunwindstack_result.regs();

  // If both pc and $rsp do not change during unwinding, there was an unwinding error.
  if ((new_regs.pc() == rip && new_regs.sp() == rsp) || libunwindstack_result.frames().empty()) {
    // If the error was because the stack sample was too small, the user can act and increase the
    // stack size. So we report that case separately.
    if (stack_size > stack_dump_size_) {
      return orbit_grpc_protos::Callstack::kStackTopForDwarfUnwindingTooSmall;
    }
    return orbit_grpc_protos::Callstack::kStackTopDwarfUnwindingError;
  }

  uint64_t new_rbp = new_regs[unwindstack::X86_64_REG_RBP];
  // $rbp changed during unwinding (case (2)), i.e. either it was a valid frame pointer and thus
  // the callchain is already correct, or it was modified as general purpose register (unwinding
  // error).
  if (new_rbp != rbp) {
    // If the $rbp after unwinding is below the sampled $rbp, the sampled $rbp could not be a
    // valid frame pointer (remember the stack grows downwards).
    // Note that, in addition to this check, we also check if the complete callchain is in
    // executable code in the UprobesUnwindingVisitor.
    if (new_rbp < rbp) {
      return orbit_grpc_protos::Callstack::kFramePointerUnwindingError;
    }
    return orbit_grpc_protos::Callstack::kComplete;
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
  std::shared_ptr<unwindstack::MapInfo> caller_map_info =
      current_maps->Find(libunwindstack_leaf_caller_pc);
  if (caller_map_info == nullptr || (caller_map_info->flags() & PROT_EXEC) == 0) {
    // As above, if the error was because the stack sample was too small, the user can act and
    // increase the stack size. So we report that case separately.
    if (stack_size > stack_dump_size_) {
      return orbit_grpc_protos::Callstack::kStackTopForDwarfUnwindingTooSmall;
    }
    return orbit_grpc_protos::Callstack::kStackTopDwarfUnwindingError;
  }

  result.push_back(libunwindstack_leaf_caller_pc);

  for (size_t i = 2; i < original_callchain.size(); ++i) {
    result.push_back(original_callchain[i]);
  }

  event_data->SetIps(result);

  return orbit_grpc_protos::Callstack::kComplete;
}

template orbit_grpc_protos::Callstack::CallstackType
LeafFunctionCallManager::PatchCallerOfLeafFunctionImpl<CallchainSamplePerfEventData>(
    const CallchainSamplePerfEventData* event_data, LibunwindstackMaps* current_maps,
    LibunwindstackUnwinder* unwinder);

template orbit_grpc_protos::Callstack::CallstackType
LeafFunctionCallManager::PatchCallerOfLeafFunctionImpl<SchedWakeupWithCallchainPerfEventData>(
    const SchedWakeupWithCallchainPerfEventData* event_data, LibunwindstackMaps* current_maps,
    LibunwindstackUnwinder* unwinder);

template orbit_grpc_protos::Callstack::CallstackType
LeafFunctionCallManager::PatchCallerOfLeafFunctionImpl<SchedSwitchWithCallchainPerfEventData>(
    const SchedSwitchWithCallchainPerfEventData* event_data, LibunwindstackMaps* current_maps,
    LibunwindstackUnwinder* unwinder);
}  //  namespace orbit_linux_tracing