// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LEAF_FUNCTION_CALL_MANAGER_H_
#define LINUX_TRACING_LEAF_FUNCTION_CALL_MANAGER_H_

#include <stdint.h>

#include "GrpcProtos/capture.pb.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "PerfEvent.h"

namespace orbit_linux_tracing {

// This class provides the `PatchCallerOfLeafFunction` method to fix a frame-pointer based
// callchain, where the leaf function does not have frame-pointers. Note that this is wrapped in a
// class to allow tests to mock this implementation.
class LeafFunctionCallManager {
 public:
  explicit LeafFunctionCallManager(uint16_t stack_dump_size) : stack_dump_size_{stack_dump_size} {}
  virtual ~LeafFunctionCallManager() = default;

  // Computes the actual caller of a leaf function (that may not have frame-pointers) based on
  // libunwindstack and modifies the given callchain event, if needed.
  // In case of any unwinding error (either from libunwindstack or in the frame-pointer based
  // callchain), the respective `CallstackType` will be returned and the event remains untouched.
  // If the innermost frame has frame-pointers, this function will return `kComplete` and keeps the
  // callchain event untouched.
  // Otherwise, that is if the caller of the leaf function is missing and there are no unwinding
  // errors, the callchain event gets updated, such that it contains the missing caller, and
  // `kComplete` will be returned.
  // Note that the address of the caller address is computed by decreasing the return address by
  // one in libunwindstack, to match the format of perf_event_open.
  virtual orbit_grpc_protos::Callstack::CallstackType PatchCallerOfLeafFunction(
      const CallchainSamplePerfEventData* event_data, LibunwindstackMaps* current_maps,
      LibunwindstackUnwinder* unwinder) {
    return PatchCallerOfLeafFunctionImpl(event_data, current_maps, unwinder);
  }

  virtual orbit_grpc_protos::Callstack::CallstackType PatchCallerOfLeafFunction(
      const SchedWakeupWithCallchainPerfEventData* event_data, LibunwindstackMaps* current_maps,
      LibunwindstackUnwinder* unwinder) {
    return PatchCallerOfLeafFunctionImpl(event_data, current_maps, unwinder);
  }

  virtual orbit_grpc_protos::Callstack::CallstackType PatchCallerOfLeafFunction(
      const SchedSwitchWithCallchainPerfEventData* event_data, LibunwindstackMaps* current_maps,
      LibunwindstackUnwinder* unwinder) {
    return PatchCallerOfLeafFunctionImpl(event_data, current_maps, unwinder);
  }

 private:
  // Let's unwind one frame using libunwindstack. With that unwinding step, the registers will get
  // updated and we can detect if $rbp was modified.
  // (1) If $rbp did not change: We are in a leaf function, which has not modified $rbp. The
  // leaf's
  //     caller is missing in the callchain and needs to be patched in. The updated $rip (pc) from
  //     the unwinding step contains the leaf's caller.
  // (2) If $rbp was modified, this can either be:
  //     (a) We are in a non-leaf function and the callchain is already correct.
  //     (b) We are in a leaf function that modified $rbp. The complete callchain is broken and
  //     should
  //         be reported as unwinding error.
  // As libunwindstack does not report us the canonical frame address (CFA) from an unwinding
  // step, we cannot differentiate between (2a) and (2b) reliably. However, we do perform the
  // following validity checks (for the reasoning remember that the stack grows downwards): (I) If
  // the CFA is computed using $rbp + 16, we know the $rbp was correct, i.e. case (2a) (II)  If
  // $rbp is below $rsp, $rbp is not a frame pointer, i.e. case (2b) (III) If $rbp moves up the
  // stack after unwinding, the sampled $rbp is not a frame pointer (2b)
  //
  // Note that we cannot simply set libunwindstack to unwind always two frames and compare the
  // outer frame with the respective one in the callchain carried by the perf_event_open event, as
  // in case of uprobes overriding the return addresses, both addresses would be identical even if
  // the actual addresses (after uprobe patching) are not. More (internal) documentation on this
  // in: go/stadia-orbit-leaf-frame-pointer
  template <typename CallchainPerfEventDataT>
  orbit_grpc_protos::Callstack::CallstackType PatchCallerOfLeafFunctionImpl(
      const CallchainPerfEventDataT* event_data, LibunwindstackMaps* current_maps,
      LibunwindstackUnwinder* unwinder);

  uint16_t stack_dump_size_;
};

}  //  namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LEAF_FUNCTION_CALL_MANAGER_H_
