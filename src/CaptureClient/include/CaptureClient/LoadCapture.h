// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_LOAD_CAPTURE_H_
#define CAPTURE_CLIENT_LOAD_CAPTURE_H_

#include <atomic>

#include "CaptureClient/CaptureListener.h"
#include "CaptureFile/CaptureFile.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_client {

// TODO(b/234110675) Add a smoke test
[[nodiscard]] ErrorMessageOr<CaptureListener::CaptureOutcome> LoadCapture(
    CaptureListener* listener, orbit_capture_file::CaptureFile* capture_file,
    std::atomic<bool>* capture_loading_cancellation_requested);

}  // namespace orbit_capture_client
#endif  // CAPTURE_CLIENT_LOAD_CAPTURE_H_
