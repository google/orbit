// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "MetricsUploader/Result.h"

using orbit_metrics_uploader::Result;

extern "C" {
__declspec(dllexport) enum Result SendOrbitLogEvent(uint8_t* serialized_proto, int length) {
  return Result::kNoError;
}
}  // extern "C"
