// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TRACK_DATA_MANAGER_H_
#define CLIENT_DATA_TRACK_DATA_MANAGER_H_

#include <absl/synchronization/mutex.h>

#include <vector>

#include "ClientData/TrackData.h"

namespace orbit_client_data {

// Creates and stores TrackData in a thread-safe way.
// Note that this class does not provide thread-safe access to ThreadData itself.
class TrackDataManager final {
 public:
  TrackDataManager() = default;

  [[nodiscard]] std::pair<uint64_t, TrackData*> CreateTrackData() {
    absl::MutexLock lock(&mutex_);
    uint64_t id = track_data_.size();
    track_data_.emplace_back(std::make_unique<TrackData>());
    return std::make_pair(id, track_data_.at(id).get());
  }

 private:
  mutable absl::Mutex mutex_;
  std::vector<std::unique_ptr<TrackData>> track_data_ GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TRACK_DATA_MANAGER_H_
