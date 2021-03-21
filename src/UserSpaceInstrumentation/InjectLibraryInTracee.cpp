// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "InjectLibraryInTracee.h"

#include <absl/base/casts.h>
#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <csignal>
#include <string>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "ElfUtils/ElfFile.h"
#include "ElfUtils/LinuxMap.h"
#include "MachineCode.h"
#include "OrbitBase/Logging.h"
#include "RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_elf_utils::ElfFile;
using orbit_elf_utils::ReadModules;

// Size of the small amount of memory we need in the tracee to write machine code into.
constexpr uint64_t kCodeScratchPadSize = 1024;

constexpr const char* kLibcSoname = "libc.so.6";
constexpr const char* kLibdlSoname = "libdl.so.2";
constexpr const char* kDlopenInLibdl = "dlopen";
constexpr const char* kDlopenInLibc = "__libc_dlopen_mode";
constexpr const char* kDlsymInLibdl = "dlsym";
constexpr const char* kDlsymInLibc = "__libc_dlsym";
constexpr const char* kDlcloseInLibdl = "dlclose";
constexpr const char* kDlcloseInLibc = "__libc_dlclose";

// In certain error conditions the tracee is damaged and we don't try to recover from that. We just
// abort with a fatal log message. None of these errors are expected to occur in operation
// obvioulsy. That's what the *OrDie methods below are for.
void FreeMemoryOrDie(pid_t pid, uint64_t address_code, uint64_t size) {
  auto result = FreeInTracee(pid, address_code, size);
  FAIL_IF(result.has_error(), "Unable to free previously allocated memory in tracee: \"%s\"",
          result.error().message());
}

void RestoreRegistersOrDie(RegisterState& register_state) {
  auto result = register_state.RestoreRegisters();
  FAIL_IF(result.has_error(), "Unable to restore register state in tracee: \"%s\"",
          result.error().message());
}

uint64_t GetReturnValueOrDie(pid_t pid) {
  RegisterState return_value_registers;
  auto result_return_value = return_value_registers.BackupRegisters(pid);
  FAIL_IF(result_return_value.has_error(), "Unable to read registers after function called :\"%s\"",
          result_return_value.error().message());
  return return_value_registers.GetGeneralPurposeRegisters()->x86_64.rax;
}

// Copies `code` to `address_code` in the tracee and executes it. The code segment has to end with
// an `int3`. Takes care of backup and restore of register state in the tracee and also deallocates
// the memory at `address_code` afterwards. The return value is the content of eax after the
// execution finished.
ErrorMessageOr<uint64_t> ExecuteOrDie(pid_t pid, uint64_t address_code, uint64_t memory_size,
                                      const MachineCode& code) {
  auto result_write_code = WriteTraceesMemory(pid, address_code, code.GetResultAsVector());
  if (result_write_code.has_error()) {
    FreeMemoryOrDie(pid, address_code, memory_size);
    return result_write_code.error();
  }

  // Backup registers.
  RegisterState original_registers;
  OUTCOME_TRY(original_registers.BackupRegisters(pid));

  RegisterState registers_set_rip = original_registers;
  registers_set_rip.GetGeneralPurposeRegisters()->x86_64.rip = address_code;
  OUTCOME_TRY(registers_set_rip.RestoreRegisters());
  if (ptrace(PTRACE_CONT, pid, 0, 0) != 0) {
    FATAL("Unable to continue tracee with PTRACE_CONT.");
  }
  int status = 0;
  pid_t waited = waitpid(pid, &status, 0);
  if (waited != pid || !WIFSTOPPED(status) || WSTOPSIG(status) != SIGTRAP) {
    FATAL(
        "Failed to wait for sigtrap after PTRACE_CONT. Expected pid: %d Pid returned from waitpid: "
        "%d status: %u, WIFSTOPPED: %u, WSTOPSIG: %u",
        pid, waited, status, WIFSTOPPED(status), WSTOPSIG(status));
  }

  const uint64_t return_value = GetReturnValueOrDie(pid);

  // Clean up memory and registers.
  RestoreRegistersOrDie(original_registers);
  FreeMemoryOrDie(pid, address_code, memory_size);
  return return_value;
}

// Returns the absolute virtual address of a function in a module of a process as
// FindFunctionAddress does but accepts a fallback symbol if the primary one cannot be resolved.
ErrorMessageOr<uint64_t> FindFunctionAddressWithFallback(pid_t pid, std::string_view function,
                                                         std::string_view module,
                                                         std::string_view fallback_function,
                                                         std::string_view fallback_module) {
  ErrorMessageOr<uint64_t> primary_address_or_error = FindFunctionAddress(pid, function, module);
  if (primary_address_or_error.has_value()) {
    return primary_address_or_error.value();
  }
  ErrorMessageOr<uint64_t> fallback_address_or_error =
      FindFunctionAddress(pid, fallback_function, fallback_module);
  if (fallback_address_or_error.has_value()) {
    return fallback_address_or_error.value();
  }

  return ErrorMessage(absl::StrFormat(
      "Failed to load symbol \"%s\" from module \"%s\" with error: \"%s\"\nAnd also "
      "failed to load fallback symbol \"%s\" from module \"%s\" with error: \"%s\"",
      function, module, primary_address_or_error.error().message(), fallback_function,
      fallback_module, fallback_address_or_error.error().message()));
}

}  // namespace

[[nodiscard]] ErrorMessageOr<void*> DlopenInTracee(pid_t pid, std::filesystem::path path,
                                                   uint32_t flag) {
  // Figure out address of dlopen.
  OUTCOME_TRY(address_dlopen, FindFunctionAddressWithFallback(pid, kDlopenInLibdl, kLibdlSoname,
                                                              kDlopenInLibc, kLibcSoname));

  // Allocate small memory area in the tracee. This is used for the code and the path name.
  const uint64_t path_length = path.string().length() + 1;  // Include terminating zero.
  const uint64_t memory_size = kCodeScratchPadSize + path_length;
  OUTCOME_TRY(address_code, AllocateInTracee(pid, 0, memory_size));

  // Write the name of the .so into memory at address_code with offset of kCodeScratchPadSize.
  std::vector<uint8_t> path_as_vector(path_length);
  memcpy(path_as_vector.data(), path.c_str(), path_length);
  const uint64_t address_so_path = address_code + kCodeScratchPadSize;
  auto result_write_path = WriteTraceesMemory(pid, address_so_path, path_as_vector);
  if (result_write_path.has_error()) {
    FreeMemoryOrDie(pid, address_code, memory_size);
    return result_write_path.error();
  }

  // We want to do the following in the tracee:
  // return_value = dlopen(path, flag);
  // The calling convention is to put the parameters in registers rdi and rsi.
  // So the address of the file path goes to rdi. The flag argument goes into rsi. Then we load the
  // address of dlopen into rax and do the call. Assembly in Intel syntax (destination first),
  // machine code on the right:

  // movabsq rdi, address_so_path     48 bf address_so_path
  // movl esi, flag                   be flag
  // movabsq rax, address_dlopen      48 b8 address_dlopen
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xbf})
      .AppendImmediate64(address_so_path)
      .AppendBytes({0xbe})
      .AppendImmediate32(flag)
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(address_dlopen)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  auto return_value_or_error = ExecuteOrDie(pid, address_code, memory_size, code);
  if (return_value_or_error.has_error()) {
    FreeMemoryOrDie(pid, address_code, memory_size);
    return return_value_or_error.error();
  }
  return absl::bit_cast<void*>(return_value_or_error.value());
}

[[nodiscard]] ErrorMessageOr<void*> DlsymInTracee(pid_t pid, void* handle,
                                                  std::string_view symbol) {
  // Figure out address of dlsym.
  OUTCOME_TRY(address_dlsym, FindFunctionAddressWithFallback(pid, kDlsymInLibdl, kLibdlSoname,
                                                             kDlsymInLibc, kLibcSoname));

  // Allocate small memory area in the tracee. This is used for the code and the symbol name.
  const size_t symbol_name_length = symbol.length() + 1;  // include terminating zero
  const uint64_t memory_size = kCodeScratchPadSize + symbol_name_length;
  OUTCOME_TRY(address_code, AllocateInTracee(pid, 0, memory_size));

  // Write the name of symbol into memory at address_code with offset of kCodeScratchPadSize.
  std::vector<uint8_t> symbol_name_as_vector(symbol_name_length, 0);
  memcpy(symbol_name_as_vector.data(), symbol.data(), symbol.length());
  const uint64_t address_symbol_name = address_code + kCodeScratchPadSize;
  auto result_write_symbol_name =
      WriteTraceesMemory(pid, address_symbol_name, symbol_name_as_vector);
  if (result_write_symbol_name.has_error()) {
    FreeMemoryOrDie(pid, address_code, memory_size);
    return result_write_symbol_name.error();
  }

  // We want to do the following in the tracee:
  // return_value = dlsym(handle, symbol);
  // The calling convention is to put the parameters in registers rdi and rsi.
  // So the handle goes to rdi and the address of the symbol name goes to rsi. Then we load the
  // address of dlsym into rax and do the call. Assembly in Intel syntax (destination first),
  // machine code on the right:

  // movabsq rdi, handle              48 bf handle
  // movabsq rsi, address_symbol_name 48 be address_symbol_name
  // movabsq rax, address_dlsym       48 b8 address_dlsym
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xbf})
      .AppendImmediate64(absl::bit_cast<uint64_t>(handle))
      .AppendBytes({0x48, 0xbe})
      .AppendImmediate64(address_symbol_name)
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(address_dlsym)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  auto return_value_or_error = ExecuteOrDie(pid, address_code, memory_size, code);
  if (return_value_or_error.has_error()) {
    FreeMemoryOrDie(pid, address_code, memory_size);
    return return_value_or_error.error();
  }
  return absl::bit_cast<void*>(return_value_or_error.value());
}

[[nodiscard]] ErrorMessageOr<void> DlcloseInTracee(pid_t pid, void* handle) {
  // Figure out address of dlclose.
  OUTCOME_TRY(address_dlclose, FindFunctionAddressWithFallback(pid, kDlcloseInLibdl, kLibdlSoname,
                                                               kDlcloseInLibc, kLibcSoname));

  // Allocate small memory area in the tracee.
  OUTCOME_TRY(address_code, AllocateInTracee(pid, 0, kCodeScratchPadSize));

  // We want to do the following in the tracee:
  // dlclose(handle);
  // The calling convention is to put the parameter in registers rdi. Then we load the address of
  // address_dlclose into rax and do the call. Assembly in Intel syntax (destination first), machine
  // code on the right:

  // movabsq rdi, handle              48 bf handle
  // movabsq rax, address_dlclose     48 b8 address_dlclose
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xbf})
      .AppendImmediate64(absl::bit_cast<uint64_t>(handle))
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(address_dlclose)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  auto return_value_or_error = ExecuteOrDie(pid, address_code, kCodeScratchPadSize, code);
  if (return_value_or_error.has_error()) {
    FreeMemoryOrDie(pid, address_code, kCodeScratchPadSize);
    return return_value_or_error.error();
  }

  return outcome::success();
}

ErrorMessageOr<uint64_t> FindFunctionAddress(pid_t pid, std::string_view function_name,
                                             std::string_view module_soname) {
  auto modules = ReadModules(pid);
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

  auto elf_file = ElfFile::Create(module_file_path);
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