// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/Result.h"

namespace orbit_metrics_uploader {

std::string GetErrorMessage(Result result) {
  switch (result) {
    case kMetricsUploaderServiceNotStarted:
      return "Metrics uploader service was not started";
    case kSdkConfigNotLoaded:
      return "SDK config was not loaded";
    case kCannotMarshalLogEvent:
      return "Can't marshall log event";
    case kCannotQueueLogEvent:
      return "Can't queue log event";
    case kClientNotInitialized:
      return "Uploader client wasn't initialized. StartUploaderClient() should be called before "
             "sending log events.";
    case kCannotUnmarshalLogEvent:
      return "Can't unmarshal OrbitLogEvent proto";
    default:
      return "No errors";
  }
}

}  // namespace orbit_metrics_uploader
