// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "PresentEventManager.h"

#include "OrbitBase/Logging.h"

namespace orbit_gl {

using orbit_grpc_protos::PresentEvent;

static std::vector<std::string> TrackNames(std::string_view suffix) {
  std::vector<std::string> track_names(PresentEvent::Source_ARRAYSIZE);
  track_names[PresentEvent::kD3d9] = absl::StrFormat("D3d9%s", suffix);
  track_names[PresentEvent::kDxgi] = absl::StrFormat("Dxgi%s", suffix);
  return track_names;
}

const std::string& PresentEventManager::GetFpsTrackNameFromSource(PresentEvent::Source source) {
  static std::vector<std::string> track_names = TrackNames(" FPS");
  ORBIT_CHECK(static_cast<size_t>(source) < track_names.size());
  ORBIT_CHECK(!track_names[source].empty());
  return track_names[source];
}

const std::string& PresentEventManager::GetFrameTimeTrackNameFromSource(
    PresentEvent::Source source) {
  static std::vector<std::string> track_names = TrackNames(" Frame Time [ms]");
  ORBIT_CHECK(static_cast<size_t>(source) < track_names.size());
  ORBIT_CHECK(!track_names[source].empty());
  return track_names[source];
}

std::optional<uint64_t> PresentEventManager::ExchangeLastTimeStampForSource(
    PresentEvent::Source source, uint64_t new_timestamp_ns) {
  absl::MutexLock lock(&mutex_);
  auto it = source_to_last_timestamp_ns_.find(source);
  if (it != source_to_last_timestamp_ns_.end()) {
    uint64_t last_timestamp_ns = it->second;
    ORBIT_CHECK(new_timestamp_ns >= last_timestamp_ns);
    it->second = new_timestamp_ns;
    return last_timestamp_ns;
  }

  // This is the first time we register a timestamp.
  source_to_last_timestamp_ns_.emplace(source, new_timestamp_ns);
  return std::nullopt;
}

}  // namespace orbit_gl
