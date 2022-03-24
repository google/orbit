// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/CallstackType.h"

#include "OrbitBase/Logging.h"

using orbit_grpc_protos::Callstack;

namespace orbit_client_data {

std::string CallstackTypeToString(CallstackType callstack_type) {
  switch (callstack_type) {
    case CallstackType::kComplete:
      return "Complete";
    case CallstackType::kDwarfUnwindingError:
      return "DWARF unwinding error";
    case CallstackType::kFramePointerUnwindingError:
      return "Frame pointer unwinding error";
    case CallstackType::kInUprobes:
      return "Callstack inside uprobes (kernel)";
    case CallstackType::kInUserSpaceInstrumentation:
      return "Callstack inside user-space instrumentation";
    case CallstackType::kCallstackPatchingFailed:
      return "Callstack patching failed";
    case CallstackType::kStackTopForDwarfUnwindingTooSmall:
      return "Collected raw stack is too small";
    case CallstackType::kStackTopDwarfUnwindingError:
      return "DWARF unwinding error in inner frame";
    case CallstackType::kFilteredByMajorityOutermostFrame:
      return "Unknown unwinding error";
  }
  ORBIT_UNREACHABLE();
}

std::string CallstackTypeToDescription(CallstackType callstack_type) {
  switch (callstack_type) {
    case CallstackType::kComplete:
      return "Unwinding succeeded.";
    case CallstackType::kDwarfUnwindingError:
      return "DWARF unwinding failed on the collected sample.";
    case CallstackType::kFramePointerUnwindingError:
      return "Frame pointer unwinding failed on the collected sample. Likely, the callstack "
             "contains a function not compiled with frame pointers (-fno-omit-frame-pointer).";
    case CallstackType::kInUprobes:
      return "The collected callstack falls inside uprobes (kernel) code.";
    case CallstackType::kInUserSpaceInstrumentation:
      return "The collected callstack falls inside the user-space instrumentation code.";
    case CallstackType::kCallstackPatchingFailed:
      return "Repairing a callstack that contains dynamically instrumented functions failed.";
    case CallstackType::kStackTopForDwarfUnwindingTooSmall:
      return "The collected raw stack is too small to unwind. You can increase the size to collect "
             "in the capture options.";
    case CallstackType::kStackTopDwarfUnwindingError:
      return "DWARF unwinding the inner frame to patch a leaf function (-momit-leaf-frame-pointer) "
             "failed.";
    case CallstackType::kFilteredByMajorityOutermostFrame:
      return "The outermost frame does not match the majority for this thread, so the callstack "
             "has been marked as unwound incorrectly.";
  }
  ORBIT_UNREACHABLE();
}

CallstackType GrpcCallstackTypeToCallstackType(
    orbit_grpc_protos::Callstack::CallstackType callstack_type) {
  switch (callstack_type) {
    case Callstack::kComplete:
      return CallstackType::kComplete;
    case Callstack::kDwarfUnwindingError:
      return CallstackType::kDwarfUnwindingError;
    case Callstack::kFramePointerUnwindingError:
      return CallstackType::kFramePointerUnwindingError;
    case Callstack::kInUprobes:
      return CallstackType::kInUprobes;
    case Callstack::kInUserSpaceInstrumentation:
      return CallstackType::kInUserSpaceInstrumentation;
    case Callstack::kCallstackPatchingFailed:
      return CallstackType::kCallstackPatchingFailed;
    case Callstack::kStackTopForDwarfUnwindingTooSmall:
      return CallstackType::kStackTopForDwarfUnwindingTooSmall;
    case Callstack::kStackTopDwarfUnwindingError:
      return CallstackType::kStackTopDwarfUnwindingError;
    case orbit_grpc_protos::
        Callstack_CallstackType_Callstack_CallstackType_INT_MIN_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
    case orbit_grpc_protos::
        Callstack_CallstackType_Callstack_CallstackType_INT_MAX_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
  }
  ORBIT_UNREACHABLE();
}

}  // namespace orbit_client_data
