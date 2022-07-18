// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CLIENT_DATA_WINE_SYSCALL_HANDLING_METHOD_H_
#define CLIENT_DATA_WINE_SYSCALL_HANDLING_METHOD_H_

#include <cstdint>

namespace orbit_client_data {

// With newer Wine versions, unwinding will fail after `__wine_syscall_dispatcher`
// (see go/unwinding_wine_syscall_dispatcher). The main reason for failing is that the "syscall"
// implementation of Wine operates on a different stack than the "Windows user-space" stack.
// We see two conceptual mitigations for those unwinding errors:
//   1. Tell the unwinder to stop at `__wine_syscall_dispatcher` and report a "complete" callstack.
//   2. Apply (expensive) special handling to retrieve a copy of the "Windows user-space" stack.
// For older Wine versions, no special handling is needed at all.
// We give the user the ability to choose between the different options of mitigation. This enum
// encodes the respective options.
enum class WineSyscallHandlingMethod : uint8_t {
  kNoSpecialHandling,
  kStopUnwinding,
  kRecordUserStack
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_WINE_SYSCALL_HANDLING_METHOD_H_
