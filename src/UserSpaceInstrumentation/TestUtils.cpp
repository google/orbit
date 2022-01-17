// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TestUtils.h"

#include <absl/strings/str_cat.h>
#include <capstone/capstone.h>

#include <string>

#include "ObjectUtils/Address.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_user_space_instrumentation {

static ErrorMessageOr<AddressRange> FindFunctionAbsoluteAddressInModule(
    std::string_view function_name, std::string_view module_file_path,
    AddressRange module_address_range) {
  OUTCOME_TRY(auto&& elf_file, orbit_object_utils::CreateElfFile(module_file_path));
  OUTCOME_TRY(auto&& syms, elf_file->LoadDebugSymbols());
  for (const auto& sym : syms.symbol_infos()) {
    if (sym.name() == function_name) {
      const uint64_t address = orbit_object_utils::SymbolVirtualAddressToAbsoluteAddress(
          sym.address(), module_address_range.start, syms.load_bias(),
          elf_file->GetExecutableSegmentOffset());
      const uint64_t size = sym.size();
      return AddressRange{address, address + size};
    }
  }

  return ErrorMessage{"No matching function found."};
}

AddressRange GetFunctionAbsoluteAddressRangeOrDie(std::string_view function_name) {
  auto modules_or_error = orbit_object_utils::ReadModules(getpid());
  ORBIT_CHECK(!modules_or_error.has_error());
  auto& modules = modules_or_error.value();

  // We check the main module first because it's most likely to find the function there.
  const auto main_module = std::find_if(modules.begin(), modules.end(), [&](const auto& module) {
    return module.file_path() == orbit_base::GetExecutablePath();
  });

  if (main_module != modules.end()) {
    auto result = FindFunctionAbsoluteAddressInModule(
        function_name, main_module->file_path(),
        AddressRange{main_module->address_start(), main_module->address_end()});
    if (result.has_value()) return result.value();
  }

  // If the main module doesn't contain the function we will look through all the other modules.
  for (const auto& module : modules) {
    auto result = FindFunctionAbsoluteAddressInModule(
        function_name, module.file_path(),
        AddressRange{module.address_start(), module.address_end()});
    if (result.has_value()) return result.value();
  }

  ORBIT_FATAL("GetFunctionAbsoluteAddressRangeOrDie hasn't found a function '%s'", function_name);
}

static ErrorMessageOr<AddressRange> FindFunctionRelativeAddressInModule(
    std::string_view function_name, std::string_view module_file_path) {
  OUTCOME_TRY(auto&& elf_file, orbit_object_utils::CreateElfFile(module_file_path));
  OUTCOME_TRY(auto&& syms, elf_file->LoadDebugSymbols());
  for (const auto& sym : syms.symbol_infos()) {
    if (sym.name() == function_name) {
      return AddressRange(sym.address(), sym.address() + sym.size());
    }
  }

  return ErrorMessage{"No matching function found."};
}

FunctionLocation FindFunctionOrDie(std::string_view function_name) {
  auto modules_or_error = orbit_object_utils::ReadModules(getpid());
  ORBIT_CHECK(!modules_or_error.has_error());
  auto& modules = modules_or_error.value();

  // We check the main module first because it's most likely to find the function there.
  const auto main_module = std::find_if(modules.begin(), modules.end(), [&](const auto& module) {
    return module.file_path() == orbit_base::GetExecutablePath();
  });

  if (main_module != modules.end()) {
    auto result = FindFunctionRelativeAddressInModule(function_name, main_module->file_path());
    if (result.has_value()) return FunctionLocation{main_module->file_path(), result.value()};
  }

  // If the main module doesn't contain the function we will look through all the other modules.
  for (const auto& module : modules) {
    auto result = FindFunctionRelativeAddressInModule(function_name, module.file_path());
    if (result.has_value()) return FunctionLocation{module.file_path(), result.value()};
  }
  ORBIT_FATAL("FindFunctionOrDie hasn't found a function '%s'", function_name);
}

void DumpDisassembly(const std::vector<uint8_t>& code, uint64_t start_address) {
  // Init Capstone disassembler.
  csh capstone_handle = 0;
  cs_err error_code = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
  ORBIT_CHECK(error_code == CS_ERR_OK);
  error_code = cs_option(capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
  ORBIT_CHECK(error_code == CS_ERR_OK);
  orbit_base::unique_resource close_on_exit{
      &capstone_handle, [](csh* capstone_handle) { cs_close(capstone_handle); }};

  cs_insn* instruction = nullptr;
  const size_t count = cs_disasm(capstone_handle, static_cast<const uint8_t*>(code.data()),
                                 code.size(), start_address, 0, &instruction);
  size_t i;
  for (i = 0; i < count; i++) {
    std::string machine_code;
    for (int j = 0; j < instruction[i].size; j++) {
      machine_code =
          absl::StrCat(machine_code, j == 0 ? absl::StrFormat("%#0.2x", instruction[i].bytes[j])
                                            : absl::StrFormat(" %0.2x", instruction[i].bytes[j]));
    }
    ORBIT_LOG("%#x:\t%-12s %s , %s", instruction[i].address, instruction[i].mnemonic,
              instruction[i].op_str, machine_code);
  }
  // Print out the next offset, after the last instruction.
  ORBIT_LOG("%#x:", instruction[i - 1].address + instruction[i - 1].size);
  cs_free(instruction, count);
}

}  // namespace orbit_user_space_instrumentation