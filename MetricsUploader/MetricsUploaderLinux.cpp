// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/MetricsUploader.h"

namespace orbit_metrics_uploader {

MetricsUploader::MetricsUploader() = default;
MetricsUploader::~MetricsUploader() = default;
MetricsUploader::MetricsUploader(MetricsUploader&& other) = default;
MetricsUploader& MetricsUploader::operator=(MetricsUploader&& other) = default;

ErrorMessageOr<MetricsUploader> MetricsUploader::CreateMetricsUploader(std::string) {
  return ErrorMessage("MetricsUploader is not implemented on Linux");
}

}  // namespace orbit_metrics_uploader
