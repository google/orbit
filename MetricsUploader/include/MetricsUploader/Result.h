// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_METRICS_UPLOADER_RESULT_H_
#define ORBIT_METRICS_UPLOADER_RESULT_H_

#include <string>

namespace orbit_metrics_uploader {

// This enum represents the errors that could happen inside the metrics uploader client library, and
// should be in sync with the enum in the library itself.
enum Result {
  kNoError,
  kMetricsUploaderServiceNotStarted,
  kSdkConfigNotLoaded,
  kCannotMarshalLogEvent,
  kCannotQueueLogEvent,
  kClientNotInitialized,
  kCannotUnmarshalLogEvent
};

std::string GetErrorMessage(Result result);

}  // namespace orbit_metrics_uploader

#endif  // ORBIT_METRICS_UPLOADER_RESULT_H_
