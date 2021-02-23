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

namespace orbit_base {

using unique_fd = unique_resource<int, void (*)(int)>;

// The following functions provide convenience wrappers for working with file
// descriptors without having having to do TEMP_FAILURE_RETRY and
// different subroutines for Windows/Linux. They also translate errors to
// ErrorMessageOr<T>.

ErrorMessageOr<unique_fd> OpenFileForReading(const std::filesystem::path& path);

ErrorMessageOr<unique_fd> OpenFileForWriting(const std::filesystem::path& path);

ErrorMessageOr<void> WriteFully(const unique_fd& fd, std::string_view content);

// Tries to read 'size' bytes from the file to the buffer, returns actual
// number of bytes read. Note that the return value is less then size in
// the case when end of file was encountered.
//
// Use this function only for reading from files. This function is not supposed to be
// used for non-blocking reads from sockets/pipes - it does not handle EAGAIN.
ErrorMessageOr<size_t> ReadFully(const unique_fd& fd, void* buffer, size_t size);

}  // namespace orbit_base

#endif  // ORBIT_BASE_FILE_H_
