// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ReadSeccompModeOfThread.h"

#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <linux/seccomp.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

std::optional<int> ReadSeccompModeOfThread(pid_t tid) {
  std::string status_file_path = absl::StrFormat("/proc/%d/status", tid);
  ErrorMessageOr<std::string> status_content_or_error =
      orbit_base::ReadFileToString(status_file_path);
  if (status_content_or_error.has_error()) {
    ORBIT_ERROR("%s", status_content_or_error.error().message());
    return std::nullopt;
  }

  constexpr const char* kSeccompPrefix = "Seccomp:";
  std::istringstream status_content_stream{status_content_or_error.value()};
  std::string status_line;
  while (std::getline(status_content_stream, status_line)) {
    if (!absl::StartsWith(status_line, kSeccompPrefix)) continue;

    std::vector<std::string_view> seccomp_tokens =
        absl::StrSplit(status_line, absl::ByAnyChar(": \t"), absl::SkipWhitespace{});
    if (seccomp_tokens.size() < 2) break;

    int seccomp_mode = -1;
    if (!absl::SimpleAtoi(seccomp_tokens[1], &seccomp_mode)) break;

    if (seccomp_mode != SECCOMP_MODE_DISABLED && seccomp_mode != SECCOMP_MODE_STRICT &&
        seccomp_mode != SECCOMP_MODE_FILTER) {
      break;
    }
    return seccomp_mode;
  }

  ORBIT_ERROR("Could not read seccomp mode of thread %d", tid);
  return std::nullopt;
}

}  // namespace orbit_user_space_instrumentation
