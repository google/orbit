// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/TimestampIntervalSet.h"

#include <algorithm>

#include "OrbitBase/Logging.h"

namespace orbit_client_data {

void TimestampIntervalSet::Add(uint64_t start_inclusive, uint64_t end_exclusive) {
  ORBIT_CHECK(start_inclusive < end_exclusive);

  uint64_t new_start = start_inclusive;
  auto it = intervals_.upper_bound(start_inclusive);
  if (it != intervals_.begin()) {
    // start_inclusive is not before the first interval.
    --it;
    ORBIT_CHECK(start_inclusive >= it->start_inclusive());
    if (start_inclusive <= it->end_exclusive() && end_exclusive <= it->end_exclusive()) {
      // The new interval is completely included in one of the existing intervals.
      return;
    }
    if (start_inclusive <= it->end_exclusive()) {
      // The new interval intersects or is adjacent to the current existing interval.
      new_start = it->start_inclusive();
      it = intervals_.erase(it);
    } else {
      ++it;
    }
  }

  uint64_t new_end = end_exclusive;
  while (it != intervals_.end() && end_exclusive >= it->start_inclusive()) {
    // The new interval intersects or is adjacent to the current existing interval.
    new_end = std::max(new_end, it->end_exclusive());
    it = intervals_.erase(it);
  }

  intervals_.emplace(new_start, new_end);
}

TimestampIntervalSet::const_iterator TimestampIntervalSet::LowerBound(uint64_t timestamp) const {
  auto it = intervals_.lower_bound(timestamp);
  if (it == intervals_.begin()) {
    return it;
  }

  --it;
  if (timestamp < it->end_exclusive()) {
    return it;
  }
  ++it;
  return it;
}

}  // namespace orbit_client_data
