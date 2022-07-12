// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "PresentEventManager.h"

#include "OrbitBase/Logging.h"

namespace orbit_gl {

using orbit_grpc_protos::PresentEvent;

const char* PresentEventManager::GetFpsTrackNameFromSource(PresentEvent::Source source) {
  switch (source) {
    case PresentEvent::kD3d9:
      return "D3d9 FPS";
    case PresentEvent::kDxgi:
      return "Dxgi FPS";
    default:
      ORBIT_UNREACHABLE();
  }
}

const char* PresentEventManager::GetFrameTimeTrackNameFromSource(PresentEvent::Source source) {
  switch (source) {
    case PresentEvent::kD3d9:
      return "D3d9 Frame Time [ms]";
    case PresentEvent::kDxgi:
      return "Dxgi Frame Time [ms]";
    default:
      ORBIT_UNREACHABLE();
  }
}

// Replace the value of "last_timestamp_ns_by_type_" with a new value and returns the old value.
std::optional<uint64_t> PresentEventManager::ExchangeLastTimeStampForSource(
    PresentEvent::Source source, uint64_t new_timestamp_ns) {
  absl::MutexLock lock(&mutex_);
  auto it = last_timestamp_ns_by_type_.find(source);
  if(it != last_timestamp_ns_by_type_.end()) {
    uint64_t last_timestamp_ns = it->second;
    ORBIT_CHECK(new_timestamp_ns >= last_timestamp_ns);
    it->second = new_timestamp_ns;
    return last_timestamp_ns;
  }

  last_timestamp_ns_by_type_.emplace(source, new_timestamp_ns);
  return std::nullopt;
}

}  // namespace orbit_gl
