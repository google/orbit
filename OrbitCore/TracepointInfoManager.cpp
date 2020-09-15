// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracepointInfoManager.h"

bool TracepointInfoManager::AddUniqueTracepointEventInfo(
    uint64_t key, orbit_grpc_protos::TracepointInfo tracepoint) {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  if (unique_tracepoint_.contains(key)) {
    return false;
  }
  unique_tracepoint_.emplace(key, tracepoint);
  return true;
}

orbit_grpc_protos::TracepointInfo* TracepointInfoManager::Get(uint64_t key) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  auto it = unique_tracepoint_.find(key);
  if (it != unique_tracepoint_.end()) {
    return const_cast<orbit_grpc_protos::TracepointInfo*>(&it->second);
  } else {
    return nullptr;
  }
}

bool TracepointInfoManager::Contains(uint64_t key) const {
  absl::MutexLock lock{&unique_tracepoints_mutex_};
  return unique_tracepoint_.contains(key);
}
