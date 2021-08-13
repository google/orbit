// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_STATUS_H_
#define ORBIT_SSH_STATUS_H_

#include <absl/status/status.h>

#include <optional>
#include <string>

#include "OrbitBase/Result.h"
#include "OrbitSsh/Error.h"

namespace orbit_ssh {

enum class Status { kSuccess, kTryAgain };

[[nodiscard]] inline bool ShouldITryAgain(const ErrorMessageOr<Status>& result) {
  return result.has_value() && result.value() == Status::kTryAgain;
}

template <typename T>
[[nodiscard]] bool ShouldITryAgain(const ErrorMessageOr<std::optional<T>>& result) {
  return result.has_value() && !result.value().has_value();
}

[[nodiscard]] inline ErrorMessageOr<Status> CreateStatus(int rc) {
  if (rc == 0) return Status::kSuccess;
  if (rc == LIBSSH2_ERROR_EAGAIN) return Status::kTryAgain;
  return ErrorMessage{orbit_ssh::make_error_code(static_cast<Error>(rc)).message()};
}

}  // namespace orbit_ssh

#endif  // ORBIT_SSH_STATUS_H_