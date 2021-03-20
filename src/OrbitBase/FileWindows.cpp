// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <windows.h>

#include "OrbitBase/File.h"

namespace orbit_base {

ErrorMessageOr<std::filesystem::path> GetFilePathFromFd(const unique_fd& fd) {
  std::array<char, 1024> file_path_buffer{};
  HANDLE handle = absl::bit_cast<HANDLE>(_get_osfhandle(fd.get()));
  size_t name_length = GetFinalPathNameByHandleA(handle, file_path_buffer.data(),
                                                 file_path_buffer.size(), FILE_NAME_NORMALIZED);
  if (name_length == 0) {
    DWORD error = GetLastError();
    std::array<char, 1024> message_buffer{};
    DWORD result =
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error,
                       0, message_buffer.data(), message_buffer.size(), nullptr);
    if (result == 0) {
      return ErrorMessage{absl::StrFormat(
          "GetFileInformationByHandleEx with error code %d (unable to get error message)", error)};
    }

    return ErrorMessage{message_buffer.data()};
  }

  CHECK(name_length < file_path_buffer.size());
  file_path_buffer[name_length] = 0;

  const char* result = file_path_buffer.data();
  // apparently
  // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfinalpathnamebyhandlea#remarks
  if (result[0] == '\\' && result[1] == '\\' && result[2] == '?' && result[3] == '\\') {
    result += 4;
  }

  return std::filesystem::path{result};
}

}  // namespace orbit_base