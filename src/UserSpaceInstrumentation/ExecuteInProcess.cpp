// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/ExecuteInProcess.h"

#include <absl/base/casts.h>
#include <absl/types/span.h>

#include <memory>

#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "MachineCode.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

// Size of the small amount of memory we need in the tracee to write machine code into.
constexpr uint64_t kCodeScratchPadSize = 1024;

}  // namespace

ErrorMessageOr<uint64_t> ExecuteInProcess(pid_t pid, void* function_address, uint64_t param_0,
                                          uint64_t param_1, uint64_t param_2, uint64_t param_3,
                                          uint64_t param_4, uint64_t param_5) {
  // The input parameters go into rdi, rsi, rdx, rcx, r8 and r9 in that order. Write
  // `function_address` into rax, call the function and hit a breakpoint. The return value (if
  // any) will be in rax.
  // movabsq rdi, param_0             48 bf param_0
  // movabsq rsi, param_1             48 be param_1
  // movabsq rdx, param_2             48 ba param_2
  // movabsq rcx, param_3             48 b9 param_3
  // movabsq  r8, param_4             49 b8 param_4
  // movabsq  r9, param_5             49 b9 param_5
  // movabsq rax, function_address    48 b8 function_address
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xbf})
      .AppendImmediate64(param_0)
      .AppendBytes({0x48, 0xbe})
      .AppendImmediate64(param_1)
      .AppendBytes({0x48, 0xba})
      .AppendImmediate64(param_2)
      .AppendBytes({0x48, 0xb9})
      .AppendImmediate64(param_3)
      .AppendBytes({0x49, 0xb8})
      .AppendImmediate64(param_4)
      .AppendBytes({0x49, 0xb9})
      .AppendImmediate64(param_5)
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(absl::bit_cast<uint64_t>(function_address))
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  OUTCOME_TRY(auto&& memory, AutomaticMemoryInTracee::Create(pid, 0, kCodeScratchPadSize));

  OUTCOME_TRY(auto&& return_value, ExecuteMachineCode(*memory, code));

  return return_value;
}

ErrorMessageOr<uint64_t> ExecuteInProcess(pid_t pid,
                                          absl::Span<const orbit_grpc_protos::ModuleInfo> modules,
                                          void* library_handle, std::string_view function,
                                          uint64_t param_0, uint64_t param_1, uint64_t param_2,
                                          uint64_t param_3, uint64_t param_4, uint64_t param_5) {
  OUTCOME_TRY(auto&& function_address, DlsymInTracee(pid, modules, library_handle, function));
  return ExecuteInProcess(pid, function_address, param_0, param_1, param_2, param_3, param_4,
                          param_5);
}

ErrorMessageOr<uint64_t> ExecuteInProcessWithMicrosoftCallingConvention(
    pid_t pid, void* function_address, uint64_t param_0, uint64_t param_1, uint64_t param_2,
    uint64_t param_3) {
  // In the Microsoft x64 ABI, if the first four parameters are integer they go in RCX, RDX, R8, R9.
  // See https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention#parameter-passing
  // The return value is in RAX.
  // See https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention#return-values
  // movabsq rcx, param_0             48 b9 param_0
  // movabsq rdx, param_1             48 ba param_1
  // movabsq r8,  param_2             49 b8 param_2
  // movabsq r9,  param_3             49 b9 param_3
  // movabsq rax, function_address    48 b8 function_address
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xb9})
      .AppendImmediate64(param_0)
      .AppendBytes({0x48, 0xba})
      .AppendImmediate64(param_1)
      .AppendBytes({0x49, 0xb8})
      .AppendImmediate64(param_2)
      .AppendBytes({0x49, 0xb9})
      .AppendImmediate64(param_3)
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(absl::bit_cast<uint64_t>(function_address))
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  OUTCOME_TRY(auto&& memory, AutomaticMemoryInTracee::Create(pid, 0, kCodeScratchPadSize));

  OUTCOME_TRY(auto&& return_value, ExecuteMachineCode(*memory, code));

  return return_value;
}

}  // namespace orbit_user_space_instrumentation