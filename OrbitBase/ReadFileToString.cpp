// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/ReadFileToString.h"

#include <absl/strings/str_format.h>

#include <fstream>

#include "OrbitBase/SafeStrerror.h"

namespace orbit_base {

ErrorMessageOr<std::string> ReadFileToString(const std::filesystem::path& file_name) {
  std::ifstream file_stream(file_name);
  if (file_stream.fail()) {
    return ErrorMessage(
        absl::StrFormat("Unable to read file %s: %s", file_name.string(), SafeStrerror(errno)));
  }

  return std::string{std::istreambuf_iterator<char>{file_stream}, std::istreambuf_iterator<char>{}};
}

}  // namespace orbit_base