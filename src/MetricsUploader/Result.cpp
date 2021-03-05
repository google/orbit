// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/Result.h"

namespace orbit_metrics_uploader {

std::string GetErrorMessage(Result result) {
  switch (result) {
    case kCannotOpenConnection:
      return "Connection to metrics uploader wasn't opened";
    case kSdkConfigNotLoaded:
      return "SDK config was not loaded";
    case kCannotMarshalLogEvent:
      return "Can't marshall log event";
    case kUnknownEventType:
      return "Event type is unknown";
    case kCannotQueueLogEvent:
      return "Can't queue log event";
    case kConnectionNotInitialized:
      return "Connection wasn't initialized. SetupConnection() should be called before sending log "
             "events.";
    case kCannotUnmarshalLogEvent:
      return "Can't unmarshal OrbitLogEvent proto";
    case kCannotCloseConnection:
      return "Connection to metrics uploader wasn't closed";
    case kNoError:
      return "No errors";
    case kUnknownStatusCode:
      return "Unknown status code (metrics_uploader_client.dll does not know the status code)";
    case kStatusCodeMismatch:
      return "The sent status code cannot be matched to an internal status code";
    default:
      return "Unexpected error occured. See metrics_uploader_client log for details";
  }
}

}  // namespace orbit_metrics_uploader
