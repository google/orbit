// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_LIBSSH2_UTILS_H_
#define ORBIT_SSH_LIBSSH2_UTILS_H_

#include <libssh2.h>

#include <string>
#include <utility>

namespace orbit_ssh {

std::pair<int, std::string> LibSsh2SessionLastError(LIBSSH2_SESSION* session);

}  // namespace orbit_ssh

#endif  // ORBIT_SSH_LIBSSH2_UTILS_H_
