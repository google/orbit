// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_UTILS_H_
#define LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_UTILS_H_

#include <absl/strings/match.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <filesystem>
#include <string>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing_integration_tests {

[[nodiscard]] inline bool IsRunningAsRoot() { return geteuid() == 0; }

[[nodiscard]] inline bool CheckIsRunningAsRoot() {
  if (IsRunningAsRoot()) {
    return true;
  }

  ORBIT_ERROR("Root required for this test");
  return false;
}

[[nodiscard]] bool CheckIsStadiaInstance();

[[nodiscard]] std::filesystem::path GetExecutableBinaryPath(pid_t pid);

[[nodiscard]] orbit_grpc_protos::ModuleSymbols GetExecutableBinaryModuleSymbols(pid_t pid);

[[nodiscard]] orbit_grpc_protos::ModuleInfo GetExecutableBinaryModuleInfo(pid_t pid);

}  // namespace orbit_linux_tracing_integration_tests

#endif  // LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_UTILS_H_
