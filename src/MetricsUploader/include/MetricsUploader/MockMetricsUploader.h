// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>

#include "MetricsUploader/MetricsUploader.h"
#include "MetricsUploader/orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

class MockMetricsUploader : public MetricsUploader {
 public:
  MOCK_METHOD(bool, SendLogEvent, (OrbitLogEvent::LogEventType /*log_event_type*/), (override));
  MOCK_METHOD(bool, SendLogEvent,
              (OrbitLogEvent::LogEventType /*log_event_type*/,
               std::chrono::milliseconds /*event_duration*/),
              (override));
  MOCK_METHOD(bool, SendLogEvent,
              (OrbitLogEvent::LogEventType /*log_event_type*/,
               std::chrono::milliseconds /*event_duration*/,
               OrbitLogEvent::StatusCode /*status_code*/),
              (override));
  MOCK_METHOD(bool, SendCaptureEvent,
              (OrbitCaptureData /*capture data*/, OrbitLogEvent::StatusCode /*status_code*/),
              (override));
  MOCK_METHOD(bool, SendSymbolLoadEvent,
              (OrbitPerModuleSymbolLoadData /*symbol_load_data*/,
               std::chrono::milliseconds /*event_duration*/,
               OrbitLogEvent::StatusCode /*status_code*/),
              (override));
};

}  // namespace orbit_metrics_uploader