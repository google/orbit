// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "IntegrationTestUtils.h"

#include <filesystem>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_linux_tracing_integration_tests {

[[nodiscard]] static std::string ReadUnameKernelRelease() {
  utsname utsname{};
  int uname_result = uname(&utsname);
  ORBIT_CHECK(uname_result == 0);
  return utsname.release;
}

bool CheckIsStadiaInstance() {
  std::string release = ReadUnameKernelRelease();
  if (absl::StrContains(release, "-ggp-")) {
    return true;
  }

  ORBIT_ERROR("Stadia instance required for this test (but kernel release is \"%s\")", release);
  return false;
}

std::filesystem::path GetExecutableBinaryPath(pid_t pid) {
  auto error_or_executable_path =
      orbit_base::GetExecutablePath(orbit_base::FromNativeProcessId(pid));
  ORBIT_CHECK(error_or_executable_path.has_value());
  return error_or_executable_path.value();
}

orbit_grpc_protos::ModuleSymbols GetExecutableBinaryModuleSymbols(pid_t pid) {
  const std::filesystem::path& executable_path = GetExecutableBinaryPath(pid);

  auto error_or_elf_file = orbit_object_utils::CreateElfFile(executable_path.string());
  ORBIT_CHECK(error_or_elf_file.has_value());
  const std::unique_ptr<orbit_object_utils::ElfFile>& elf_file = error_or_elf_file.value();

  auto error_or_module = elf_file->LoadDebugSymbolsAsProto();
  ORBIT_CHECK(error_or_module.has_value());
  return error_or_module.value();
}

orbit_grpc_protos::ModuleInfo GetExecutableBinaryModuleInfo(pid_t pid) {
  auto error_or_module_infos = orbit_object_utils::ReadModules(pid);
  ORBIT_CHECK(error_or_module_infos.has_value());
  const std::vector<orbit_grpc_protos::ModuleInfo>& module_infos = error_or_module_infos.value();

  const std::filesystem::path& executable_path = GetExecutableBinaryPath(pid);

  const orbit_grpc_protos::ModuleInfo* executable_module_info = nullptr;
  for (const auto& module_info : module_infos) {
    if (module_info.file_path() == executable_path) {
      executable_module_info = &module_info;
      break;
    }
  }
  ORBIT_CHECK(executable_module_info != nullptr);
  return *executable_module_info;
}

}  // namespace orbit_linux_tracing_integration_tests
