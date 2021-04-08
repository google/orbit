// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/MetricsUploader.h"
#include "MetricsUploader/MetricsUploaderStub.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_metrics_uploader {

std::unique_ptr<MetricsUploader> MetricsUploader::CreateMetricsUploader(std::string) {
  ERROR("MetricsUploader is not implemented on Linux");
  return std::make_unique<MetricsUploaderStub>();
}

ErrorMessageOr<std::string> GenerateUUID() {
  return ErrorMessage("UUID generation is not implemented on Linux");
}

}  // namespace orbit_metrics_uploader
