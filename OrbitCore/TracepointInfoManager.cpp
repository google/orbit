// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointInfoManager.h"

bool TracepointInfoManager::AddUniqueTracepointEvent(uint64_t key,
                                                     orbit_grpc_protos::TracepointInfo tracepoint) {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  unique_tracepoint_.emplace(key, tracepoint);
  return true;
}

void TracepointInfoManager::AddTracepointEvent(
    orbit_client_protos::TracepointEvent tracepoint_event) {
  uint64_t hash = tracepoint_event.tracepoint_hash();
  CHECK(Contains(hash));
  tracepoint_events_.push_back(std::move(tracepoint_event));
}

std::optional<orbit_grpc_protos::TracepointInfo> TracepointInfoManager::Get(uint64_t key) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  auto it = unique_tracepoint_.find(key);
  if (it != unique_tracepoint_.end()) {
    return it->second;
  } else {
    return std::optional<orbit_grpc_protos::TracepointInfo>{};
  }
}

bool TracepointInfoManager::Contains(uint64_t key) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  return unique_tracepoint_.contains(key);
}
