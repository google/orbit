// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/DllInjection.h"

#include <absl/base/casts.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/OpenProcess.h"
#include "WindowsUtils/SafeHandle.h"
#include "WindowsUtils/WriteProcessMemory.h"

// clang-format off
#include <windows.h>
#include <processthreadsapi.h>
// clang-format on

namespace orbit_windows_utils {

namespace {

ErrorMessageOr<uint64_t> RemoteWrite(HANDLE process_handle, absl::Span<const char> buffer) {
  // Allocate memory in target process.
  LPVOID base_address = VirtualAllocEx(process_handle, /*lpAddress=*/0, buffer.size(),
                                       MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (base_address == nullptr) {
    return orbit_base::GetLastErrorAsErrorMessage("VirtualAllocEx");
  }

  // Write in allocated remote memory.
  OUTCOME_TRY(WriteProcessMemory(process_handle, base_address, buffer));

  return absl::bit_cast<uint64_t>(base_address);
}

template <typename T>
ErrorMessageOr<T> RemoteRead(HANDLE process_handle, uint64_t base_address) {
  size_t number_of_bytes_read = 0;
  T result = {0};
  if (!ReadProcessMemory(process_handle, absl::bit_cast<LPCVOID>(base_address), &result, sizeof(T),
                         &number_of_bytes_read)) {
    return orbit_base::GetLastErrorAsErrorMessage("ReadProcessMemory");
  }

  if (number_of_bytes_read != sizeof(T)) {
    return ErrorMessage("Number of bytes read is not equal to requested size");
  }

  return result;
}

ErrorMessageOr<std::string> RemoteReadString(HANDLE process_handle, uint64_t base_address) {
  std::string result;
  // We don't know the string length, but it is null terminated. Read one character at a time.
  for (size_t i = 0;; ++i) {
    OUTCOME_TRY(auto character, RemoteRead<char>(process_handle, base_address + i));
    if (character == '\0') break;
    result.push_back(character);
  }
  return result;
}

ErrorMessageOr<void> ValidatePath(std::filesystem::path path) {
  if (!std::filesystem::exists(path))
    return ErrorMessage(absl::StrFormat("Path does not exist: %s", path.string()));
  return outcome::success();
}

ErrorMessageOr<Module> FindModule(uint32_t pid, std::string_view module_name) {
  std::vector<Module> modules = ListModules(pid);
  std::vector<Module> result;
  for (const Module& module : modules) {
    if (absl::EqualsIgnoreCase(module.name, module_name)) {
      result.push_back(module);
    }
  }
  if (result.size() == 1) return result[0];
  if (result.size() > 1) {
    return ErrorMessage(
        absl::StrFormat("Multiple modules with the name \"%s\" found", module_name));
  }
  return ErrorMessage("Could not find module in target process");
}

ErrorMessageOr<Module> FindModuleWithRetries(uint32_t pid, std::string_view module_name,
                                             uint32_t num_retries,
                                             uint64_t time_between_retries_ms) {
  while (true) {
    ErrorMessageOr<Module> result = FindModule(pid, module_name);
    if (result.has_value() || num_retries-- <= 0) return result;
    std::this_thread::sleep_for(std::chrono::milliseconds(time_between_retries_ms));
  }
}

ErrorMessageOr<void> EnsureModuleIsNotAlreadyLoaded(uint32_t pid, std::string_view module_name) {
  ErrorMessageOr<Module> module = FindModule(pid, module_name);
  if (module.has_value()) {
    return ErrorMessage(
        absl::StrFormat("Module \"%s\" is already loaded in process %u", module_name, pid));
  }
  return outcome::success();
}

ErrorMessageOr<void> InjectDllInternal(uint32_t pid, const std::filesystem::path& dll_path) {
  const std::string dll_name = dll_path.string();
  ORBIT_SCOPED_TIMED_LOG("Injecting dll \"%s\" in process %u", dll_name, pid);

  // Inject dll by calling "LoadLibraryA" in remote process with the name of our dll as parameter.
  OUTCOME_TRY(CreateRemoteThread(pid, "kernel32.dll", "LoadLibraryA", dll_name));

  // Find injected dll in target process. Allow for retries as the loading might take some time.
  constexpr uint32_t kNumRetries = 10;
  constexpr uint64_t kTimeBetweenRetriesMs = 250;
  OUTCOME_TRY(Module module, FindModuleWithRetries(pid, dll_path.filename().string(), kNumRetries,
                                                   kTimeBetweenRetriesMs));

  ORBIT_LOG("Module \"%s\" successfully injected in process %u", dll_name, pid);
  return outcome::success();
}

}  // namespace

ErrorMessageOr<void> InjectDll(uint32_t pid, std::filesystem::path dll_path) {
  OUTCOME_TRY(ValidatePath(dll_path));
  OUTCOME_TRY(EnsureModuleIsNotAlreadyLoaded(pid, dll_path.filename().string()));
  OUTCOME_TRY(InjectDllInternal(pid, dll_path));
  return outcome::success();
}

ErrorMessageOr<void> InjectDllIfNotLoaded(uint32_t pid, std::filesystem::path dll_path) {
  OUTCOME_TRY(ValidatePath(dll_path));

  if (EnsureModuleIsNotAlreadyLoaded(pid, dll_path.filename().string()).has_error()) {
    // Dll is already loaded, no more work to do.
    return outcome::success();
  }

  OUTCOME_TRY(InjectDllInternal(pid, dll_path));
  return outcome::success();
}

ErrorMessageOr<void> CreateRemoteThread(uint32_t pid, std::string_view module_name,
                                        std::string_view function_name,
                                        absl::Span<const char> parameter) {
  OUTCOME_TRY(uint64_t function_address, GetRemoteProcAddress(pid, module_name, function_name));
  OUTCOME_TRY(SafeHandle safe_handle,
              OpenProcess(PROCESS_ALL_ACCESS, /*inherit_handle=*/false, pid));
  HANDLE handle = *safe_handle;

  // Write parameter to remote process memory.
  uint64_t parameter_address = 0;
  if (!parameter.empty()) {
    OUTCOME_TRY(uint64_t address, RemoteWrite(handle, parameter));
    parameter_address = address;
  }

  if (!::CreateRemoteThread(handle, /*lpThreadAttributes=*/0, /*dwStackSize=*/0,
                            absl::bit_cast<LPTHREAD_START_ROUTINE>(function_address),
                            absl::bit_cast<LPVOID>(parameter_address),
                            /*dwCreationFlags=*/0, /*lpThreadId=*/0)) {
    return orbit_base::GetLastErrorAsErrorMessage("CreateRemoteThread");
  }

  return outcome::success();
}

// Parse remote module's MS-DOS, NT, and optional headers in order to locate the
// IMAGE_EXPORT_DIRECTORY structure and find the requested function address.
// See https://docs.microsoft.com/en-us/windows/win32/debug/pe-format for more info.
ErrorMessageOr<uint64_t> GetRemoteProcAddress(uint32_t pid, std::string_view module_name,
                                              std::string_view function_name) {
  OUTCOME_TRY(Module module, FindModule(pid, module_name));
  uint64_t module_base = module.address_start;

  OUTCOME_TRY(SafeHandle safe_handle,
              OpenProcess(PROCESS_ALL_ACCESS, /*inherit_handle=*/false, pid));
  HANDLE handle = *safe_handle;

  OUTCOME_TRY(auto image_dos_header, RemoteRead<IMAGE_DOS_HEADER>(handle, module_base));
  if (image_dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
    return ErrorMessage("IMAGE_DOS_SIGNATURE not found");
  }

  const uint64_t nt_headers_address = module_base + image_dos_header.e_lfanew;
  OUTCOME_TRY(auto signature, RemoteRead<DWORD>(handle, nt_headers_address));
  if (signature != IMAGE_NT_SIGNATURE) {
    return ErrorMessage("IMAGE_NT_SIGNATURE not found");
  }

  OUTCOME_TRY(auto image_file_header,
              RemoteRead<IMAGE_FILE_HEADER>(handle, nt_headers_address + sizeof(signature)));

  IMAGE_DATA_DIRECTORY export_directory = {0};
  uint64_t optional_header_address =
      module_base + image_dos_header.e_lfanew + sizeof(signature) + sizeof(image_file_header);
  static_assert(sizeof(IMAGE_OPTIONAL_HEADER64) != sizeof(IMAGE_OPTIONAL_HEADER32));
  if (image_file_header.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64)) {
    // 64 bit optional header.
    OUTCOME_TRY(auto optional_header_64,
                RemoteRead<IMAGE_OPTIONAL_HEADER64>(handle, optional_header_address));
    if (optional_header_64.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
      return ErrorMessage("IMAGE_NT_OPTIONAL_HDR64_MAGIC not found");
    }
    export_directory.VirtualAddress =
        optional_header_64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    export_directory.Size = optional_header_64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
  } else if (image_file_header.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32)) {
    // 32 bit optional header.
    OUTCOME_TRY(auto optional_header_32,
                RemoteRead<IMAGE_OPTIONAL_HEADER32>(handle, optional_header_address));
    if (optional_header_32.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
      return ErrorMessage("IMAGE_NT_OPTIONAL_HDR32_MAGIC not found");
    }
    export_directory.VirtualAddress =
        optional_header_32.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    export_directory.Size = optional_header_32.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
  } else {
    return ErrorMessage("Unexpected optional header size");
  }

  // VirtualAddress actually represents a relative virtual address here.
  const uint64_t export_directory_rva = export_directory.VirtualAddress;
  if (export_directory_rva == 0) {
    return ErrorMessage("Invalid export directory address");
  }
  const uint64_t export_directory_address = module_base + export_directory_rva;
  OUTCOME_TRY(auto image_export_directory,
              RemoteRead<IMAGE_EXPORT_DIRECTORY>(handle, export_directory_address));

  const uint64_t address_of_functions = module_base + image_export_directory.AddressOfFunctions;
  const uint64_t address_of_names = module_base + image_export_directory.AddressOfNames;
  const uint64_t address_of_ordinals = module_base + image_export_directory.AddressOfNameOrdinals;

  for (size_t i = 0; i < image_export_directory.NumberOfNames; ++i) {
    OUTCOME_TRY(auto name_rva,
                RemoteRead<uint32_t>(handle, address_of_names + i * sizeof(uint32_t)));
    OUTCOME_TRY(std::string read_name, RemoteReadString(handle, module_base + name_rva));

    if (absl::EqualsIgnoreCase(read_name, function_name)) {
      OUTCOME_TRY(auto ordinal, RemoteRead<WORD>(handle, address_of_ordinals + i * sizeof(WORD)));
      OUTCOME_TRY(auto function_rva,
                  RemoteRead<uint32_t>(handle, address_of_functions + ordinal * sizeof(uint32_t)));
      return module_base + function_rva;
    }
  }

  return ErrorMessage(
      absl::StrFormat("Did not find function \"%s\" in module \"%s\"", function_name, module_name));
}

}  // namespace orbit_windows_utils
