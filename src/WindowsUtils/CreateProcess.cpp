// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/CreateProcess.h"

#include <absl/strings/str_format.h>

#include <algorithm>

#include "OrbitBase/GetLastError.h"

// clang-format off
#include <Windows.h>
#include <psapi.h>
// clang-format on

namespace orbit_windows_utils {

namespace {

ErrorMessageOr<ProcessInfo> CreateProcess(const std::filesystem::path& executable_path,
                                          const std::filesystem::path& working_directory_path,
                                          const std::string_view arguments,
                                          uint32_t creation_flags) {
  if (!std::filesystem::exists(executable_path)) {
    return ErrorMessage(
        absl::StrFormat("Executable does not exist: \"%s\"", executable_path.string()));
  }

  if (!working_directory_path.empty() && !std::filesystem::exists(working_directory_path)) {
    return ErrorMessage(absl::StrFormat("Working directory does not exist: \"%s\"",
                                        working_directory_path.string()));
  }

  std::string working_directory = working_directory_path.string();
  std::string command_line = executable_path.string();
  if (!arguments.empty()) {
    command_line += absl::StrFormat(" %s", arguments);
  }

  STARTUPINFOA startup_info = {0};
  PROCESS_INFORMATION process_info = {0};
  startup_info.cb = sizeof(startup_info);

  if (CreateProcessA(/*lpApplicationName=*/nullptr, command_line.data(),
                     /*lpProcessAttributes*/ nullptr, /*lpThreadAttributes*/ nullptr,
                     /*bInheritHandles*/ FALSE, creation_flags, /*lpEnvironment*/ nullptr,
                     working_directory.empty() ? nullptr : working_directory.c_str(), &startup_info,
                     &process_info) == 0) {
    return orbit_base::GetLastErrorAsErrorMessage("CreateProcess");
  }

  // A SafeHandle makes sure "CloseHandle" is called when the ProcessInfo goes out of scope.
  return ProcessInfo{working_directory, command_line, process_info.dwProcessId,
                     SafeHandle(process_info.hProcess), SafeHandle(process_info.hThread)};
}

}  // namespace

ErrorMessageOr<ProcessInfo> CreateProcessToDebug(const std::filesystem::path& executable,
                                                 const std::filesystem::path& working_directory,
                                                 const std::string_view arguments) {
  return CreateProcess(executable, working_directory, arguments, DEBUG_ONLY_THIS_PROCESS);
}

ErrorMessageOr<ProcessInfo> CreateProcess(const std::filesystem::path& executable,
                                          const std::filesystem::path& working_directory,
                                          const std::string_view arguments) {
  return CreateProcess(executable, working_directory, arguments, /*creation_flags=*/0);
}

}  // namespace orbit_windows_utils