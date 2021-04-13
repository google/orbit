// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

#include <absl/base/casts.h>

#include <string>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "FindFunctionAddress.h"
#include "MachineCode.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_user_space_instrumentation {

namespace {

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
// obvioulsy.
void FreeMemoryOrDie(pid_t pid, uint64_t address_code, uint64_t size) {
  auto result = FreeInTracee(pid, address_code, size);
  FAIL_IF(result.has_error(), "Unable to free previously allocated memory in tracee: \"%s\"",
          result.error().message());
}

// Returns the absolute virtual address of a function in a module of a process as
// FindFunctionAddress does but accepts a fallback symbol if the primary one cannot be resolved.
ErrorMessageOr<uint64_t> FindFunctionAddressWithFallback(pid_t pid, std::string_view module,
                                                         std::string_view function,
                                                         std::string_view fallback_module,
                                                         std::string_view fallback_function) {
  ErrorMessageOr<uint64_t> primary_address_or_error = FindFunctionAddress(pid, module, function);
  if (primary_address_or_error.has_value()) {
    return primary_address_or_error.value();
  }
  ErrorMessageOr<uint64_t> fallback_address_or_error =
      FindFunctionAddress(pid, fallback_module, fallback_function);
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
  OUTCOME_TRY(address_dlopen, FindFunctionAddressWithFallback(pid, kLibdlSoname, kDlopenInLibdl,
                                                              kLibcSoname, kDlopenInLibc));

  // Allocate small memory area in the tracee. This is used for the code and the path name.
  const uint64_t path_length = path.string().length() + 1;  // Include terminating zero.
  const uint64_t memory_size = kCodeScratchPadSize + path_length;
  OUTCOME_TRY(address, AllocateInTracee(pid, 0, memory_size));
  orbit_base::unique_resource address_code{address, [&pid, &memory_size](uint64_t address) {
                                             FreeMemoryOrDie(pid, address, memory_size);
                                           }};

  // Write the name of the .so into memory at address_code with offset of kCodeScratchPadSize.
  std::vector<uint8_t> path_as_vector(path_length);
  memcpy(path_as_vector.data(), path.c_str(), path_length);
  const uint64_t address_so_path = address_code + kCodeScratchPadSize;
  auto result_write_path = WriteTraceesMemory(pid, address_so_path, path_as_vector);
  if (result_write_path.has_error()) {
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

  OUTCOME_TRY(return_value, ExecuteMachineCode(pid, address_code, code));

  return absl::bit_cast<void*>(return_value);
}

[[nodiscard]] ErrorMessageOr<void*> DlsymInTracee(pid_t pid, void* handle,
                                                  std::string_view symbol) {
  // Figure out address of dlsym.
  OUTCOME_TRY(address_dlsym, FindFunctionAddressWithFallback(pid, kLibdlSoname, kDlsymInLibdl,
                                                             kLibcSoname, kDlsymInLibc));

  // Allocate small memory area in the tracee. This is used for the code and the symbol name.
  const size_t symbol_name_length = symbol.length() + 1;  // include terminating zero
  const uint64_t memory_size = kCodeScratchPadSize + symbol_name_length;
  OUTCOME_TRY(address, AllocateInTracee(pid, 0, memory_size));
  orbit_base::unique_resource address_code{address, [&pid, &memory_size](uint64_t address) {
                                             FreeMemoryOrDie(pid, address, memory_size);
                                           }};

  // Write the name of symbol into memory at address_code with offset of kCodeScratchPadSize.
  std::vector<uint8_t> symbol_name_as_vector(symbol_name_length, 0);
  memcpy(symbol_name_as_vector.data(), symbol.data(), symbol.length());
  const uint64_t address_symbol_name = address_code + kCodeScratchPadSize;
  auto result_write_symbol_name =
      WriteTraceesMemory(pid, address_symbol_name, symbol_name_as_vector);
  if (result_write_symbol_name.has_error()) {
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

  OUTCOME_TRY(return_value, ExecuteMachineCode(pid, address_code, code));

  return absl::bit_cast<void*>(return_value);
}

[[nodiscard]] ErrorMessageOr<void> DlcloseInTracee(pid_t pid, void* handle) {
  // Figure out address of dlclose.
  OUTCOME_TRY(address_dlclose, FindFunctionAddressWithFallback(pid, kLibdlSoname, kDlcloseInLibdl,
                                                               kLibcSoname, kDlcloseInLibc));

  // Allocate small memory area in the tracee.
  OUTCOME_TRY(address, AllocateInTracee(pid, 0, kCodeScratchPadSize));
  orbit_base::unique_resource address_code{
      address, [&pid](uint64_t address) { FreeMemoryOrDie(pid, address, kCodeScratchPadSize); }};

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

  auto return_value_or_error = ExecuteMachineCode(pid, address_code, code);
  if (return_value_or_error.has_error()) {
    return return_value_or_error.error();
  }

  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation