// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsUtils/ReadProcessMemory.h"

#include <absl/strings/str_format.h>
#include <windows.h>

// Include after windows.h.
#include <memoryapi.h>

#include "OrbitBase/Logging.h"

namespace orbit_windows_utils {

ErrorMessageOr<std::string> ReadProcessMemory(uint32_t pid, const void* address, uint64_t size) {
  HANDLE process_handle = OpenProcess(PROCESS_VM_READ, /*bInheritHandle=*/FALSE, pid);
  if (process_handle == nullptr) {
    return ErrorMessage(absl::StrFormat("Could not get handle for process %u", pid));
  }

  std::string buffer(size, {});
  uint64_t num_bytes_read = 0;
  BOOL result = ::ReadProcessMemory(process_handle, address, buffer.data(), size, &num_bytes_read);

  if (result == 0) {
    return ErrorMessage(
        absl::StrFormat("Reading %u bytes at address %p of process %u", size, address, pid));
  }

  CHECK(buffer.size() == num_bytes_read);
  return buffer;
}

}  // namespace orbit_windows_utils
