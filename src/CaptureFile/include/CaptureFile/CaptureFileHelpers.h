// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_CAPTURE_FILE_HELPERS_H_
#define CAPTURE_FILE_CAPTURE_FILE_HELPERS_H_

#include <filesystem>

#include "ClientProtos/user_defined_capture_info.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_file {
ErrorMessageOr<void> WriteUserData(
    const std::filesystem::path& capture_file_path,
    const orbit_client_protos::UserDefinedCaptureInfo& user_defined_capture_info);
}  // namespace orbit_capture_file
#endif  // CAPTURE_FILE_CAPTURE_FILE_HELPERS_H_