// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TRACEPOINT_INFO_MANAGER_H_
#define ORBIT_CORE_TRACEPOINT_INFO_MANAGER_H_

#include <optional>
#include <string>

#include "BlockChain.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"
#include "tracepoint.pb.h"

class TracepointInfoManager {
 public:
  TracepointInfoManager() = default;

  bool AddUniqueTracepointEventInfo(uint64_t key, orbit_grpc_protos::TracepointInfo tracepoint);

  void AddTracepointEvent(orbit_client_protos::TracepointEventInfo tracepoint_event_info);

  [[nodiscard]] std::optional<orbit_grpc_protos::TracepointInfo> Get(uint64_t key) const;
  [[nodiscard]] bool Contains(uint64_t key) const;

 private:
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::TracepointInfo> unique_tracepoint_;
  BlockChain<orbit_client_protos::TracepointEventInfo, 16 * 1024> tracepoint_events_;
  mutable absl::Mutex unique_tracepoints_mutex_;
};

#endif  // ORBIT_CORE_TRACEPOINT_INFO_MANAGER_H
