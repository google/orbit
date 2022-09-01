// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ListModules.h"

#include <absl/base/casts.h>
#include <absl/strings/str_replace.h>
#include <windows.h>

#include <filesystem>

#include "GrpcProtos/module.pb.h"
#include "ObjectUtils/CoffFile.h"
#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/StringConversion.h"
#include "OrbitBase/UniqueResource.h"
#include "WindowsUtils/SafeHandle.h"

// clang-format off
#include <psapi.h>
#include <tlhelp32.h>
// clang-format on

using orbit_object_utils::CoffFile;

namespace orbit_windows_utils {

// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
std::vector<Module> ListModules(uint32_t pid) {
  HANDLE module_snap_handle = INVALID_HANDLE_VALUE;
  MODULEENTRY32 module_entry = {0};

  // Take a snapshot of all modules in the specified process.
  module_snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
  if (module_snap_handle == INVALID_HANDLE_VALUE) {
    ORBIT_ERROR("Calling CreateToolhelp32Snapshot for modules: %s",
                orbit_base::GetLastErrorAsString());
    return {};
  }
  SafeHandle handle_closer(module_snap_handle);

  // Retrieve information about the first module.
  module_entry.dwSize = sizeof(MODULEENTRY32);
  if (!Module32First(module_snap_handle, &module_entry)) {
    ORBIT_ERROR("Calling Module32First for pid %u", pid);
    return {};
  }

  // Walk the module list of the process.
  std::vector<Module> modules;
  do {
    std::string build_id;
    std::string module_path = orbit_base::ToStdString(module_entry.szExePath);
    module_path = absl::StrReplaceAll(module_path, {{"\\", "/"}});
    auto coff_file_or_error = orbit_object_utils::CreateCoffFile(module_path);
    std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment> sections;
    uint64_t load_bias = 0;
    if (coff_file_or_error.has_value()) {
      build_id = coff_file_or_error.value()->GetBuildId();
      load_bias = coff_file_or_error.value()->GetLoadBias();
      sections = coff_file_or_error.value()->GetObjectSegments();
    } else {
      ORBIT_ERROR(
          "Could not create Coff file for module \"%s\", build-id and sections will be empty",
          module_path);
    }

    Module& module = modules.emplace_back();
    module.name = orbit_base::ToStdString(module_entry.szModule);
    module.full_path = module_path;
    module.file_size = module_entry.modBaseSize;
    module.address_start = absl::bit_cast<uint64_t>(module_entry.modBaseAddr);
    module.address_end = module.address_start + module_entry.modBaseSize;
    module.build_id = build_id;
    module.load_bias = load_bias;
    module.sections = std::move(sections);

  } while (Module32Next(module_snap_handle, &module_entry));

  return modules;
}

}  // namespace orbit_windows_utils
