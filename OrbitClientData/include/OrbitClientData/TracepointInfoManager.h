// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TRACEPOINT_INFO_MANAGER_H_
#define ORBIT_CORE_TRACEPOINT_INFO_MANAGER_H_

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"
#include "tracepoint.pb.h"

class TracepointInfoManager {
 public:
  TracepointInfoManager() = default;

  bool AddUniqueTracepointEventInfo(uint64_t key, orbit_grpc_protos::TracepointInfo tracepoint);

  [[nodiscard]] orbit_grpc_protos::TracepointInfo Get(uint64_t key) const;
  [[nodiscard]] bool Contains(uint64_t key) const;

  void ForEachUniqueTracepointInfo(
      const std::function<void(const orbit_client_protos::TracepointInfo&)>& action) const {
    absl::MutexLock lock(&unique_tracepoints_mutex_);
    for (const auto& it : unique_tracepoint_) {
      orbit_client_protos::TracepointInfo tracepoint_info;
      tracepoint_info.set_category(it.second.category());
      tracepoint_info.set_name(it.second.name());
      tracepoint_info.set_tracepoint_info_key(it.first);
      action(tracepoint_info);
    }
  }

 private:
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::TracepointInfo> unique_tracepoint_;
  mutable absl::Mutex unique_tracepoints_mutex_;
};

#endif  // ORBIT_CORE_TRACEPOINT_INFO_MANAGER_H
