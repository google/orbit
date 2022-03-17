// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/CallstackType.h"

#include "OrbitBase/Logging.h"

using orbit_client_protos::CallstackInfo;

namespace orbit_client_data {

std::string CallstackTypeToString(CallstackInfo::CallstackType callstack_type) {
  switch (callstack_type) {
    case CallstackInfo::kComplete:
      return "Complete";
    case CallstackInfo::kDwarfUnwindingError:
      return "DWARF unwinding error";
    case CallstackInfo::kFramePointerUnwindingError:
      return "Frame pointer unwinding error";
    case CallstackInfo::kInUprobes:
      return "Callstack inside uprobes (kernel)";
    case CallstackInfo::kInUserSpaceInstrumentation:
      return "Callstack inside user-space instrumentation";
    case CallstackInfo::kCallstackPatchingFailed:
      return "Callstack patching failed";
    case CallstackInfo::kStackTopForDwarfUnwindingTooSmall:
      return "Collected raw stack is too small";
    case CallstackInfo::kStackTopDwarfUnwindingError:
      return "Dwarf unwinding error in inner frame";
    case CallstackInfo::kFilteredByMajorityOutermostFrame:
      return "Unknown unwinding error";
    case orbit_client_protos::
        CallstackInfo_CallstackType_CallstackInfo_CallstackType_INT_MIN_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
    case orbit_client_protos::
        CallstackInfo_CallstackType_CallstackInfo_CallstackType_INT_MAX_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
  }
  ORBIT_UNREACHABLE();
}

std::string CallstackTypeToDescription(CallstackInfo::CallstackType callstack_type) {
  switch (callstack_type) {
    case CallstackInfo::kComplete:
      return "Complete";
    case CallstackInfo::kDwarfUnwindingError:
      return "DWARF unwinding failed on the collected sample.";
    case CallstackInfo::kFramePointerUnwindingError:
      return "Frame pointer unwinding failed on the collected sample. Likely, the callstack "
             "contains a function not compiled with frame pointers (-fno-omit-frame-pointer)";
    case CallstackInfo::kInUprobes:
      return "The collected callstack falls inside uprobes (kernel) code.";
    case CallstackInfo::kInUserSpaceInstrumentation:
      return "The collected callstack falls inside the user-space instrumentation code.";
    case CallstackInfo::kCallstackPatchingFailed:
      return "Repairing a callstack that contains dynamically instrumented functions failed.";
    case CallstackInfo::kStackTopForDwarfUnwindingTooSmall:
      return "The collected raw stack is too small to unwind. You can increase the size to collect "
             "in the capture options.";
    case CallstackInfo::kStackTopDwarfUnwindingError:
      return "Dwarf unwinding the inner frame to patch a leaf function (-momit-leaf-frame-pointer) "
             "failed.";
    case CallstackInfo::kFilteredByMajorityOutermostFrame:
      return "The outermost frame does not match the majority for this thread, so the callstack "
             "has been marked as unwound incorrectly.";
    case orbit_client_protos::
        CallstackInfo_CallstackType_CallstackInfo_CallstackType_INT_MIN_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
    case orbit_client_protos::
        CallstackInfo_CallstackType_CallstackInfo_CallstackType_INT_MAX_SENTINEL_DO_NOT_USE_:
      ORBIT_UNREACHABLE();
  }
  ORBIT_UNREACHABLE();
}

}  // namespace orbit_client_data
