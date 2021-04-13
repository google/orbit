// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FindFunctionAddress.h"

#include <absl/strings/str_format.h>

#include <string>

#include "ElfUtils/ElfFile.h"
#include "ElfUtils/LinuxMap.h"
#include "OrbitBase/Logging.h"

namespace orbit_user_space_instrumentation {

ErrorMessageOr<uint64_t> FindFunctionAddress(pid_t pid, std::string_view module_soname,
                                             std::string_view function_name) {
  auto modules = orbit_elf_utils::ReadModules(pid);
  if (modules.has_error()) {
    return modules.error();
  }

  std::string module_file_path;
  uint64_t module_base_address = 0;
  for (const orbit_grpc_protos::ModuleInfo& module : modules.value()) {
    if (module.name() == module_soname) {
      module_file_path = module.file_path();
      module_base_address = module.address_start();
    }
  }
  if (module_file_path.empty()) {
    return ErrorMessage(
        absl::StrFormat("There is no module \"%s\" in process %d.", module_soname, pid));
  }

  OUTCOME_TRY(elf_file, orbit_elf_utils::ElfFile::Create(module_file_path));
  auto symbols = elf_file->LoadSymbolsFromDynsym();
  if (symbols.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to load symbols for module \"%s\": %s",
                                        module_soname, symbols.error().message()));
  }

  for (const orbit_grpc_protos::SymbolInfo& symbol : symbols.value().symbol_infos()) {
    if (symbol.name() == function_name) {
      return symbol.address() + module_base_address - symbols.value().load_bias();
    }
  }

  return ErrorMessage(absl::StrFormat("Unable to locate function symbol \"%s\" in module \"%s\".",
                                      function_name, module_soname));
}
}  // namespace orbit_user_space_instrumentation
