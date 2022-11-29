// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarData/GetCallstackSamplingIntervals.h"

#include <algorithm>
#include <optional>

#include "ClientData/CallstackEvent.h"
#include "MizarBase/ThreadId.h"
#include "OrbitBase/Typedef.h"

namespace orbit_mizar_data {

using ::orbit_client_data::CallstackEvent;
using ::orbit_mizar_base::TID;

std::vector<uint64_t> GetSamplingIntervalsNs(
    const absl::flat_hash_set<TID>& tids, uint64_t min_timestamp, uint64_t max_timestamp,
    const orbit_client_data::CallstackData& callstack_data) {
  std::vector<uint64_t> result;

  for (const TID tid : tids) {
    std::optional<uint64_t> previous_timestamp = std::nullopt;
    callstack_data.ForEachCallstackEventOfTidInTimeRange(
        *tid, min_timestamp, max_timestamp,
        [&result, &previous_timestamp](const CallstackEvent& event) {
          const uint64_t timestamp = event.timestamp_ns();
          if (previous_timestamp.has_value()) {
            result.push_back(timestamp - previous_timestamp.value());
          }
          previous_timestamp = timestamp;
        });
  }
  return result;
}

}  // namespace orbit_mizar_data