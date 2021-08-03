#include "TestUtils.h"

#include <absl/strings/str_cat.h>
#include <capstone/capstone.h>

#include "ObjectUtils/Address.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_user_space_instrumentation {

AddressRange GetFunctionAddressRangeInMemoryOrDie(std::string_view function_name) {
  auto modules = orbit_object_utils::ReadModules(getpid());
  CHECK(!modules.has_error());
  std::string module_file_path;
  AddressRange address_range_code(0, 0);
  for (const auto& m : modules.value()) {
    if (m.name() == orbit_base::GetExecutablePath().filename()) {
      module_file_path = m.file_path();
      address_range_code.start = m.address_start();
      address_range_code.end = m.address_end();
    }
  }
  CHECK(!module_file_path.empty());
  auto elf_file = orbit_object_utils::CreateElfFile(module_file_path);
  CHECK(!elf_file.has_error());
  auto syms = elf_file.value()->LoadDebugSymbols();
  CHECK(!syms.has_error());
  for (const auto& sym : syms.value().symbol_infos()) {
    if (sym.name() == function_name) {
      const uint64_t address = orbit_object_utils::SymbolVirtualAddressToAbsoluteAddress(
          sym.address(), address_range_code.start, syms.value().load_bias(),
          elf_file.value()->GetExecutableSegmentOffset());
      const uint64_t size = sym.size();
      return {address, address + size};
    }
  }
  CHECK(false);
  return AddressRange();
}

AddressRange GetFunctionAddressRangeInFileOrDie(std::string_view function_name) {
  auto modules = orbit_object_utils::ReadModules(getpid());
  CHECK(!modules.has_error());
  std::string module_file_path;
  for (const auto& m : modules.value()) {
    if (m.name() == orbit_base::GetExecutablePath().filename()) {
      module_file_path = m.file_path();
    }
  }
  CHECK(!module_file_path.empty());
  auto elf_file = orbit_object_utils::CreateElfFile(module_file_path);
  CHECK(!elf_file.has_error());
  auto syms = elf_file.value()->LoadDebugSymbols();
  CHECK(!syms.has_error());
  for (const auto& sym : syms.value().symbol_infos()) {
    if (sym.name() == function_name) {
      return AddressRange(sym.address(), sym.address() + sym.size());
    }
  }
  CHECK(false);
  return AddressRange();
}

void DumpDisassembly(const std::vector<uint8_t>& code, uint64_t start_address) {
  // Init Capstone disassembler.
  csh capstone_handle = 0;
  cs_err error_code = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
  CHECK(error_code == CS_ERR_OK);
  error_code = cs_option(capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
  CHECK(error_code == CS_ERR_OK);
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
    LOG("0x%llx:\t%-12s %s , %s", instruction[i].address, instruction[i].mnemonic,
        instruction[i].op_str, machine_code);
  }
  // Print out the next offset, after the last instruction.
  LOG("0x%llx:", instruction[i - 1].address + instruction[i - 1].size);
  cs_free(instruction, count);
}

}  // namespace orbit_user_space_instrumentation