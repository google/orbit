// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_EXTRACT_SIGNAL_FROM_MINIDUMP_H_
#define LINUX_CAPTURE_SERVICE_EXTRACT_SIGNAL_FROM_MINIDUMP_H_

#include <cstdint>
#include <filesystem>

#include "OrbitBase/Result.h"

namespace orbit_linux_capture_service {

// Returns the termination signal from the minidump file in `path`.
[[nodiscard]] ErrorMessageOr<int> ExtractSignalFromMinidump(const std::filesystem::path& path);

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_EXTRACT_SIGNAL_FROM_MINIDUMP_H_
