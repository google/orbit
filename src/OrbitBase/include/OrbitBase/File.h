// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_FILE_H_
#define ORBIT_BASE_FILE_H_

#include <absl/strings/str_format.h>
#include <fcntl.h>

#include <filesystem>
#include <string_view>

#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitBase/UniqueResource.h"

#if defined(__linux)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
// Windows does not have TEMP_FAILURE_RETRY - define a shortcut
#define TEMP_FAILURE_RETRY(expression) (expression)
#endif

namespace orbit_base {

using unique_fd = unique_resource<int, void (*)(int)>;

// The following functions provide convenience wrappers for working with file
// descriptors without having having to do TEMP_FAILURE_RETRY and
// different subroutines for Windows/Linux. They also translate errors to
// ErrorMessageOr<T>.

ErrorMessageOr<unique_fd> OpenFileForReading(const std::filesystem::path& path);

ErrorMessageOr<unique_fd> OpenFileForWriting(const std::filesystem::path& path);

ErrorMessageOr<void> WriteFully(const unique_fd& fd, std::string_view content);

}  // namespace orbit_base

#endif  // ORBIT_BASE_FILE_H_
