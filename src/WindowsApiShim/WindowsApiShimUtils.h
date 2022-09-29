// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_WINDOWS_API_SHIM_WINDOWS_API_SHIM_UTILS_H_
#define ORBIT_WINDOWS_API_SHIM_WINDOWS_API_SHIM_UTILS_H_

#include <iostream>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/Logging.h"

// These are not yet implemented but we generated the empty code preemptively.
#define ORBIT_TRACK_PARAM(x)
#define ORBIT_TRACK_RET(x)

#define ORBIT_SHIM_ERROR ORBIT_ERROR

// struct OrbitShimFunctionInfo {
struct TrampolineInfo {
  // Function to be called instead of the original function.
  void* detour_function;
  // Memory location of a function pointer which can be used to call the original API function from
  // within a shim function.
  void** original_function_relay;
};

[[nodiscard]] inline void* GetThreadLocalStoragePointer() {
  // Equivalent of "mov rax, gs:58h".
  return *(void**)__readgsqword(88);
}

[[nodiscard]] inline bool IsTlsValid() { return GetThreadLocalStoragePointer() != nullptr; }

#endif  // ORBIT_WINDOWS_API_SHIM_WINDOWS_API_SHIM_UTILS_H_
