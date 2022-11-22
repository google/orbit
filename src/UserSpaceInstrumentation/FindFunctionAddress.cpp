// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FindFunctionAddress.h"

#include <absl/strings/str_format.h>

#include <memory>
#include <string>

#include "GrpcProtos/symbol.pb.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "ObjectUtils/ElfFile.h"

namespace orbit_user_space_instrumentation {

ErrorMessageOr<uint64_t> FindFunctionAddress(
    const std::vector<orbit_grpc_protos::ModuleInfo>& modules, std::string_view module_soname,
    std::string_view function_name) {
  std::string module_file_path;
  uint64_t module_base_address = 0;
  for (const orbit_grpc_protos::ModuleInfo& module : modules) {
    if (module.soname() == module_soname) {
      module_file_path = module.file_path();
      module_base_address = module.address_start();
    }
  }
  if (module_file_path.empty()) {
    return ErrorMessage(
        absl::StrFormat("There is no module \"%s\" in the target process", module_soname));
  }

  OUTCOME_TRY(auto&& elf_file, orbit_object_utils::CreateElfFile(module_file_path));
  auto symbols = elf_file->LoadSymbolsFromDynsym();
  if (symbols.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to load symbols for module \"%s\": %s",
                                        module_soname, symbols.error().message()));
  }

  for (const orbit_grpc_protos::SymbolInfo& symbol : symbols.value().symbol_infos()) {
    if (symbol.demangled_name() == function_name) {
      return orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
          symbol.address(), module_base_address, elf_file->GetLoadBias(),
          elf_file->GetExecutableSegmentOffset());
    }
  }

  return ErrorMessage(absl::StrFormat(R"(Unable to locate function symbol "%s" in module "%s".)",
                                      function_name, module_soname));
}
}  // namespace orbit_user_space_instrumentation
