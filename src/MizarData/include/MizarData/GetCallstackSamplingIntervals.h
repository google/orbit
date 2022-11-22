// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_GET_CALLSTACK_SAMPLING_INTERVALS_H_
#define MIZAR_DATA_GET_CALLSTACK_SAMPLING_INTERVALS_H_

#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <vector>

#include "ClientData/CallstackData.h"
#include "MizarBase/ThreadId.h"

namespace orbit_mizar_data {

// Returns a collection of time intervals in nanos between consecutive callstack samples of the same
// thread
[[nodiscard]] std::vector<uint64_t> GetSamplingIntervalsNs(
    const absl::flat_hash_set<orbit_mizar_base::TID>& tids, uint64_t min_timestamp,
    uint64_t max_timestamp, const orbit_client_data::CallstackData& callstack_data);

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_GET_CALLSTACK_SAMPLING_INTERVALS_H_