// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/WriteStringToFile.h"

#include <absl/strings/str_format.h>
#include <stdio.h>

#include <string>

#include "OrbitBase/File.h"

namespace orbit_base {

ErrorMessageOr<void> WriteStringToFile(const std::filesystem::path& file_name,
                                       std::string_view content) {
  ErrorMessageOr<unique_fd> fd_or_error = OpenFileForWriting(file_name);
  if (fd_or_error.has_error()) {
    return fd_or_error.error();
  }

  const unique_fd& fd = fd_or_error.value();

  ErrorMessageOr<void> result = WriteFully(fd, content);
  if (result.has_error()) {
    remove(file_name.string().c_str());
    return ErrorMessage{absl::StrFormat("Unable to write to \"%s\": %s", file_name.string(),
                                        result.error().message())};
  }

  return outcome::success();
}

}  // namespace orbit_base