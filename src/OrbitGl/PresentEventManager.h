// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PRESENT_EVENT_MANAGER_H_
#define ORBIT_GL_PRESENT_EVENT_MANAGER_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include <optional>

#include "GrpcProtos/capture.pb.h"
#include "absl/container/flat_hash_map.h"

namespace orbit_gl {

// The PresentEventManager is used to categorize information about present events.
class PresentEventManager {
 public:
  PresentEventManager() = default;

  // Replace the value of source_to_last_timestamp_ns_ with a new value and return previous value.
  std::optional<uint64_t> ExchangeLastTimeStampForSource(
      orbit_grpc_protos::PresentEvent::Source source, uint64_t new_timestamp_ns);

  static const std::string& GetFrameTimeTrackNameFromSource(
      orbit_grpc_protos::PresentEvent::Source source);
  static const std::string& GetFpsTrackNameFromSource(
      orbit_grpc_protos::PresentEvent::Source source);

 private:
  absl::Mutex mutex_;
  absl::flat_hash_map<orbit_grpc_protos::PresentEvent::Source, uint64_t>
      source_to_last_timestamp_ns_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_PRESENT_EVENT_MANAGER_H_
