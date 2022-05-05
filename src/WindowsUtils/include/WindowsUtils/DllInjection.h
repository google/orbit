// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_UTILS_DLL_INJECTION_H_
#define WINDOWS_UTILS_DLL_INJECTION_H_

#include <absl/types/span.h>

#include <filesystem>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"

namespace orbit_windows_utils {

// Injects a dll in a remote process identified by "pid". Returns an ErrorMessage if the dll is
// already loaded or if the injection fails.
[[nodiscard]] ErrorMessageOr<void> InjectDll(uint32_t pid, std::filesystem::path dll_path);

// Injects a dll in a remote process identified by "pid" if it is not already loaded. The call
// succeeds if the dll is already loaded or returns an ErrorMessage if the injection fails.
[[nodiscard]] ErrorMessageOr<void> InjectDllIfNotLoaded(uint32_t pid,
                                                        std::filesystem::path dll_path);

// Create a thread in a remote process and call specified the function. The "parameter" byte buffer
// is copied to the target's memory and its address is passed to the thread function as argument.
[[nodiscard]] ErrorMessageOr<void> CreateRemoteThread(uint32_t pid, std::string_view module_name,
                                                      std::string_view function_name,
                                                      absl::Span<const char> parameter);

// Get address of function in a remote process.
[[nodiscard]] ErrorMessageOr<uint64_t> GetRemoteProcAddress(uint32_t pid,
                                                            std::string_view module_name,
                                                            std::string_view function_name);

}  // namespace orbit_windows_utils

#endif  // WINDOWS_UTILS_DLL_INJECTION_H_
