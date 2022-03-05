// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FindFunctionAddress.h"

#include <absl/strings/str_format.h>

#include <string>

#include "ObjectUtils/Address.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/LinuxMap.h"

using orbit_object_utils::DebugSymbols;
using orbit_object_utils::FunctionSymbol;

namespace orbit_user_space_instrumentation {

ErrorMessageOr<uint64_t> FindFunctionAddress(pid_t pid, std::string_view module_soname,
                                             std::string_view function_name) {
  auto modules = orbit_object_utils::ReadModules(pid);
  if (modules.has_error()) {
    return modules.error();
  }

  std::string module_file_path;
  uint64_t module_base_address = 0;
  for (const orbit_grpc_protos::ModuleInfo& module : modules.value()) {
    if (module.soname() == module_soname) {
      module_file_path = module.file_path();
      module_base_address = module.address_start();
    }
  }
  if (module_file_path.empty()) {
    return ErrorMessage(
        absl::StrFormat("There is no module \"%s\" in process %d.", module_soname, pid));
  }

  OUTCOME_TRY(auto&& elf_file, orbit_object_utils::CreateElfFile(module_file_path));
  ErrorMessageOr<DebugSymbols> symbols = elf_file->LoadSymbolsFromDynsym();
  if (symbols.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to load symbols for module \"%s\": %s",
                                        module_soname, symbols.error().message()));
  }

  for (const FunctionSymbol& symbol : symbols.value().function_symbols) {
    if (symbol.demangled_name == function_name) {
      return orbit_object_utils::SymbolVirtualAddressToAbsoluteAddress(
          symbol.relative_virtual_address, module_base_address, elf_file->GetLoadBias(),
          elf_file->GetExecutableSegmentOffset());
    }
  }

  return ErrorMessage(absl::StrFormat(R"(Unable to locate function symbol "%s" in module "%s".)",
                                      function_name, module_soname));
}
}  // namespace orbit_user_space_instrumentation
