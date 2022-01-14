// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <absl/types/span.h>

#include <cstring>
#include <string>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "ExecuteMachineCode.h"
#include "FindFunctionAddress.h"
#include "MachineCode.h"
#include "OrbitBase/File.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_user_space_instrumentation {

namespace {

// Size of the small amount of memory we need in the tracee to write machine code into.
constexpr uint64_t kCodeScratchPadSize = 1024;

constexpr const char* kLibcSoname = "libc.so.6";
constexpr const char* kLibdlSoname = "libdl.so.2";

// Represents a symbol (function) in a module.
// The member variables are string views as these are meant to be kept as constexpr constants.
struct FunctionLocatorView {
  std::string_view module_name;
  std::string_view function_name;
};

constexpr FunctionLocatorView kDlopenInLibdl{kLibdlSoname, "dlopen"};
constexpr FunctionLocatorView kDlopenInLibc{kLibcSoname, "dlopen"};
constexpr FunctionLocatorView kDlopenFallbackInLibc{kLibcSoname, "__libc_dlopen_mode"};

constexpr FunctionLocatorView kDlsymInLibdl{kLibdlSoname, "dlsym"};
constexpr FunctionLocatorView kDlsymInLibc{kLibcSoname, "dlsym"};
constexpr FunctionLocatorView kDlsymFallbackInLibc{kLibcSoname, "__libc_dlsym"};

constexpr FunctionLocatorView kDlcloseInLibdl{kLibdlSoname, "dlclose"};
constexpr FunctionLocatorView kDlcloseInLibc{kLibcSoname, "dlclose"};
constexpr FunctionLocatorView kDlcloseFallbackInLibc{kLibcSoname, "__libc_dlclose"};

// Returns the absolute virtual address of a function in a module of a process as
// FindFunctionAddress, does but accepts a list of module and function names and returns
// the address of the first found function.
ErrorMessageOr<uint64_t> FindFunctionAddressWithFallback(
    pid_t pid, absl::Span<const FunctionLocatorView> function_locators) {
  std::string error_message;

  for (const auto& function_locator : function_locators) {
    ErrorMessageOr<uint64_t> address_or_error =
        FindFunctionAddress(pid, function_locator.module_name, function_locator.function_name);
    if (address_or_error.has_value()) {
      return address_or_error.value();
    }

    if (error_message.empty()) {
      error_message.append(
          absl::StrFormat(R"(Failed to load symbol "%s" from module "%s" with error: "%s")",
                          function_locator.function_name, function_locator.module_name,
                          address_or_error.error().message()));
    } else {
      error_message.append(absl::StrFormat(
          "\nAlso failed to load fallback symbol \"%s\" from module \"%s\" with error: %s",
          function_locator.function_name, function_locator.module_name,
          address_or_error.error().message()));
    }
  }

  return ErrorMessage(error_message);
}

}  // namespace

[[nodiscard]] ErrorMessageOr<void*> DlopenInTracee(pid_t pid, const std::filesystem::path& path,
                                                   uint32_t flag) {
  // Make sure file exists.
  auto file_exists_or_error = orbit_base::FileExists(path);
  if (file_exists_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to access library at \"%s\": %s", path,
                                        file_exists_or_error.error().message()));
  }
  if (!file_exists_or_error.value()) {
    return ErrorMessage(absl::StrFormat("Library does not exist at \"%s\"", path));
  }

  // Figure out address of dlopen.
  OUTCOME_TRY(
      auto&& dlopen_address,
      FindFunctionAddressWithFallback(pid, {kDlopenInLibdl, kDlopenInLibc, kDlopenFallbackInLibc}));

  // Allocate small memory area in the tracee. This is used for the code and the path name.
  const uint64_t path_length = path.string().length() + 1;  // Include terminating zero.
  const uint64_t memory_size = kCodeScratchPadSize + path_length;
  auto code_memory_or_error = AutomaticMemoryInTracee::Create(pid, 0, memory_size);
  if (code_memory_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to allocate memory in tracee: %s",
                                        code_memory_or_error.error().message()));
  }
  auto code_memory = std::move(code_memory_or_error.value());

  // Write the name of the .so into memory at code_memory with offset of kCodeScratchPadSize.
  std::vector<uint8_t> path_as_vector(path_length);
  memcpy(path_as_vector.data(), path.c_str(), path_length);
  const uint64_t so_path_address = code_memory->GetAddress() + kCodeScratchPadSize;
  auto write_memory_result = WriteTraceesMemory(pid, so_path_address, path_as_vector);
  if (write_memory_result.has_error()) {
    return write_memory_result.error();
  }

  // We want to do the following in the tracee:
  // return_value = dlopen(path, flag);
  // The calling convention is to put the parameters in registers rdi and rsi.
  // So the address of the file path goes to rdi. The flag argument goes into rsi. Then we load the
  // address of dlopen into rax and do the call. Assembly in Intel syntax (destination first),
  // machine code on the right:

  // movabsq rdi, so_path_address     48 bf so_path_address
  // movl esi, flag                   be flag
  // movabsq rax, dlopen_address      48 b8 dlopen_address
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xbf})
      .AppendImmediate64(so_path_address)
      .AppendBytes({0xbe})
      .AppendImmediate32(flag)
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(dlopen_address)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  OUTCOME_TRY(auto&& return_value, ExecuteMachineCode(*code_memory, code));

  return absl::bit_cast<void*>(return_value);
}

[[nodiscard]] ErrorMessageOr<void*> DlsymInTracee(pid_t pid, void* handle,
                                                  std::string_view symbol) {
  // Figure out address of dlsym.
  OUTCOME_TRY(auto&& dlsym_address, FindFunctionAddressWithFallback(
                                        pid, {kDlsymInLibdl, kDlsymInLibc, kDlsymFallbackInLibc}));

  // Allocate small memory area in the tracee. This is used for the code and the symbol name.
  const size_t symbol_name_length = symbol.length() + 1;  // include terminating zero
  const uint64_t memory_size = kCodeScratchPadSize + symbol_name_length;
  auto code_memory_or_error = AutomaticMemoryInTracee::Create(pid, 0, memory_size);
  if (code_memory_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to allocate memory in tracee: %s",
                                        code_memory_or_error.error().message()));
  }
  auto code_memory = std::move(code_memory_or_error.value());

  // Write the name of symbol into memory at code_memory with offset of kCodeScratchPadSize.
  std::vector<uint8_t> symbol_name_as_vector(symbol_name_length, 0);
  memcpy(symbol_name_as_vector.data(), symbol.data(), symbol.length());
  const uint64_t symbol_name_address = code_memory->GetAddress() + kCodeScratchPadSize;
  auto write_memory_result = WriteTraceesMemory(pid, symbol_name_address, symbol_name_as_vector);
  if (write_memory_result.has_error()) {
    return write_memory_result.error();
  }

  // We want to do the following in the tracee:
  // return_value = dlsym(handle, symbol);
  // The calling convention is to put the parameters in registers rdi and rsi.
  // So the handle goes to rdi and the address of the symbol name goes to rsi. Then we load the
  // address of dlsym into rax and do the call. Assembly in Intel syntax (destination first),
  // machine code on the right:

  // movabsq rdi, handle              48 bf handle
  // movabsq rsi, symbol_name_address 48 be symbol_name_address
  // movabsq rax, dlsym_address       48 b8 dlsym_address
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xbf})
      .AppendImmediate64(absl::bit_cast<uint64_t>(handle))
      .AppendBytes({0x48, 0xbe})
      .AppendImmediate64(symbol_name_address)
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(dlsym_address)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  OUTCOME_TRY(auto&& return_value, ExecuteMachineCode(*code_memory, code));

  return absl::bit_cast<void*>(return_value);
}

[[nodiscard]] ErrorMessageOr<void> DlcloseInTracee(pid_t pid, void* handle) {
  // Figure out address of dlclose.
  OUTCOME_TRY(auto&& dlclose_address,
              FindFunctionAddressWithFallback(
                  pid, {kDlcloseInLibdl, kDlcloseInLibc, kDlcloseFallbackInLibc}));

  // Allocate small memory area in the tracee.
  auto code_memory_or_error = AutomaticMemoryInTracee::Create(pid, 0, kCodeScratchPadSize);
  if (code_memory_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to allocate memory in tracee: %s",
                                        code_memory_or_error.error().message()));
  }
  auto code_memory = std::move(code_memory_or_error.value());

  // We want to do the following in the tracee:
  // dlclose(handle);
  // The calling convention is to put the parameter in registers rdi. Then we load the address of
  // dlclose_address into rax and do the call. Assembly in Intel syntax (destination first), machine
  // code on the right:

  // movabsq rdi, handle              48 bf handle
  // movabsq rax, dlclose_address     48 b8 dlclose_address
  // call rax                         ff d0
  // int3                             cc
  MachineCode code;
  code.AppendBytes({0x48, 0xbf})
      .AppendImmediate64(absl::bit_cast<uint64_t>(handle))
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(dlclose_address)
      .AppendBytes({0xff, 0xd0})
      .AppendBytes({0xcc});

  OUTCOME_TRY(ExecuteMachineCode(*code_memory, code));

  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation
