// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/WriteStringToFile.h"

#include <absl/strings/str_format.h>

#include "OrbitBase/File.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_base {

ErrorMessageOr<void> WriteStringToFile(const std::filesystem::path& file_name,
                                       std::string_view content) {
  ErrorMessageOr<unique_fd> open_result = OpenFileForWriting(file_name);
  if (!open_result) {
    return open_result.error();
  }

  const unique_fd& fd = open_result.value();

  ErrorMessageOr<void> result = WriteFully(fd, content);
  if (!result) {
    remove(file_name.string().c_str());
    return ErrorMessage{absl::StrFormat("Unable to write to \"%s\": %s", file_name.string(),
                                        result.error().message())};
  }

  return outcome::success();
}

}  // namespace orbit_base