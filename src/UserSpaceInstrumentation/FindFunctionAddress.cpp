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

ErrorMessageOr<uint64_t> FindFunctionAddress(pid_t pid, std::string_view function_name,
                                             std::string_view module_soname) {
  auto modules = orbit_elf_utils::ReadModules(pid);
  if (modules.has_error()) {
    return modules.error();
  }

  std::string module_file_path;
  uint64_t module_base_address = 0;
  for (const auto& m : modules.value()) {
    if (m.name() == module_soname) {
      module_file_path = m.file_path();
      module_base_address = m.address_start();
    }
  }
  if (module_file_path.empty()) {
    return ErrorMessage(
        absl::StrFormat("There is no module \"%s\" in process %d.", module_soname, pid));
  }

  auto elf_file = orbit_elf_utils::ElfFile::Create(module_file_path);
  if (elf_file.has_error()) {
    return elf_file.error();
  }

  auto syms = elf_file.value()->LoadSymbolsFromDynsym();
  if (syms.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to load symbols for module \"%s\": %s",
                                        module_soname, syms.error().message()));
  }

  for (const auto& sym : syms.value().symbol_infos()) {
    if (sym.name() == function_name) {
      return sym.address() + module_base_address - syms.value().load_bias();
    }
  }

  return ErrorMessage(absl::StrFormat("Unable to locate function symbol \"%s\" in module \"%s\".",
                                      function_name, module_soname));
}
}  // namespace orbit_user_space_instrumentation
