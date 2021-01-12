// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/KnownHostsError.h"

namespace orbit_ssh {

std::string KnownHostsErrorCategory::message(int condition) const {
  switch (static_cast<KnownHostsError>(condition)) {
    case KnownHostsError::kFailure:
      return "something prevented the check to be made";
    case KnownHostsError::kNotFound:
      return "no host match was found";
    case KnownHostsError::kMismatch:
      return "host was found, but the keys didn't match!";
  }

  return "Unknown error code.";
}
}  // namespace orbit_ssh
