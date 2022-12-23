// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string_view>

namespace orbit_ssh {

// Contains the name of the environment variable that represents the home directory (depending on
// the platform). This environment variable is guaranteed to exist by the respective platform
// (unless someone messed with the environment variables).
#ifdef _WIN32
constexpr const char* kHomeEnvironmentVariable = "USERPROFILE";
#else
constexpr const char* kHomeEnvironmentVariable = "HOME";
#endif
}  // namespace orbit_ssh
