// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Api/ManualInstrumentation.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <dlfcn.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <functional>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "Api/Orbit.h"
#include "Attach.h"
#include "InjectLibraryInTracee.h"
#include "MachineCode.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "RegisterState.h"

using namespace orbit_user_space_instrumentation;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ManualInstrumentationInfo;

struct ScopeExit {
  ScopeExit(std::function<void(void)> exit_function) : exit_function(exit_function){};
  ~ScopeExit() {
    if (exit_function) exit_function();
  }
  std::function<void(void)> exit_function;
};

namespace orbit_api {
ErrorMessageOr<void> InitializeApiInTracee(const CaptureOptions& capture_options) {
  int32_t pid = capture_options.pid();

  auto attach_result = AttachAndStopProcess(pid);
  if (attach_result.has_error()) {
    return attach_result.error();
  }

  {
    // Make sure we resume the target process even on early-outs.
    ScopeExit scope_exit([pid]() {
      auto detach_result = DetachAndContinueProcess(pid);
      if (detach_result.has_error()) {
        ERROR("Resuming target process [%i]: %s", pid, detach_result.error().message());
      }
    });

    const std::string kLibName = "liborbit.so";
    auto maps_before = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));

    if (maps_before.has_error()) {
      return maps_before.error();
    }

    if (!absl::StrContains(maps_before.value(), kLibName)) {
      // Load liborbit.so into target process.
      auto result_dlopen = DlopenInTracee(pid, orbit_base::GetExecutableDir() / kLibName, RTLD_NOW);
      if (result_dlopen.has_error()) {
        return result_dlopen.error();
      }

      auto maps_after_open = orbit_base::ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
      if (maps_after_open.has_error()) {
        return maps_after_open.error();
      }

      if (absl::StrContains(maps_after_open.value(), kLibName) == false) {
        return ErrorMessage(
            absl::StrFormat("Dynamic loading of liborbit.so into target process [%i] failed", pid));
      }
    }

    // Find init function.
    const char* kInitFunction = "orbit_initialize_api";
    auto function_address_or_error = FindFunctionAddress(pid, kInitFunction, kLibName);
    CHECK(function_address_or_error.has_value());
    const uint64_t function_address = function_address_or_error.value();

    for (const ManualInstrumentationInfo& info : capture_options.manual_instrumentation_infos()) {
      // We want to do the following in the tracee:
      // return_value = orbit_initialize_api(address, api_version);
      // The calling convention is to put the parameters in registers rdi and rsi.
      // So the "address" goes to rdi and "api_version" goes to rsi. Then we load the
      // address of "orbit_api_init" into rax and do the call. Assembly in Intel syntax (destination
      // first), machine code on the right:

      // movabsq rdi, address             48 bf address
      // movabsq rsi, api_version         48 be api_version
      // movabsq rax, function_address    48 b8 function_address
      // call rax                         ff d0
      // int3                             cc
      MachineCode code;
      code.AppendBytes({0x48, 0xbf})
          .AppendImmediate64(info.api_object_address())
          .AppendBytes({0x48, 0xbe})
          .AppendImmediate64(info.api_version())
          .AppendBytes({0x48, 0xb8})
          .AppendImmediate64(function_address)
          .AppendBytes({0xff, 0xd0})
          .AppendBytes({0xcc});

      const uint64_t memory_size = code.GetResultAsVector().size();
      OUTCOME_TRY(code_address, AllocateInTracee(pid, 0, memory_size));
      auto result = ExecuteMachineCode(pid, code_address, memory_size, code);
      CHECK(!result.has_error());
    }
  }

  return outcome::success();
}

}  // namespace orbit_api
