// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/Result.h"

namespace orbit_metrics_uploader {

ErrorMessageOr<std::unique_ptr<MetricsUploader>> MetricsUploader::CreateMetricsUploader(
    std::string) {
  return ErrorMessage("MetricsUploader is not implemented on Linux");
}

ErrorMessageOr<std::string> GenerateUUID() {
  return ErrorMessage("UUID generation is not implemented on Linux");
}

}  // namespace orbit_metrics_uploader
