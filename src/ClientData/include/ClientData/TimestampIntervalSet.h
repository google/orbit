// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMESTAMP_INTERVAL_SET_H_
#define CLIENT_DATA_TIMESTAMP_INTERVAL_SET_H_

#include <stddef.h>
#include <stdint.h>

#include <set>

namespace orbit_client_data {

// This class keeps a set of intervals of uint64_t. The set is represented with the minimum number
// of sorted individual intervals.
//
// The class is introduced to keep a set of intervals of timestamps. For now, methods to add and
// query intervals are kept to only what is needed.
//
// Example usage:
//
// TimestampIntervalSet set;
// set.Add(10, 20);
// set.Add(30, 40);
// set.Add(25, 35);
// auto it = set.LowerBound(27);  // Points to [25, 40)
class TimestampIntervalSet {
 public:
  class TimestampInterval {
   public:
    TimestampInterval(uint64_t start_inclusive, uint64_t end_exclusive)
        : start_inclusive_{start_inclusive}, end_exclusive_{end_exclusive} {}

    [[nodiscard]] uint64_t start_inclusive() const { return start_inclusive_; }
    [[nodiscard]] uint64_t end_exclusive() const { return end_exclusive_; }

   private:
    uint64_t start_inclusive_;
    uint64_t end_exclusive_;
  };

 private:
  struct TimestampIntervalLess {
    // Makes this functor transparent, enabling heterogeneous lookup. See https://abseil.io/tips/144
    using is_transparent = void;

    bool operator()(const TimestampInterval& lhs, const TimestampInterval& rhs) const {
      return lhs.start_inclusive() < rhs.start_inclusive();
    }

    bool operator()(uint64_t lhs, const TimestampInterval& rhs) const {
      return lhs < rhs.start_inclusive();
    }

    bool operator()(const TimestampInterval& lhs, uint64_t rhs) const {
      return lhs.start_inclusive() < rhs;
    }
  };

  using InternalSet = std::set<TimestampInterval, TimestampIntervalLess>;

 public:
  using const_iterator = InternalSet::const_iterator;
  using const_reverse_iterator = InternalSet::const_reverse_iterator;

  // Adds the interval [start_inclusive, end_exclusive) to this `TimestampIntervalSet`.
  void Add(uint64_t start_inclusive, uint64_t end_exclusive);

  // Returns an iterator pointing to the first `TimestampInterval` that contains timestamp or starts
  // after `timestamp`.
  [[nodiscard]] const_iterator LowerBound(uint64_t timestamp) const;

  void Clear() { intervals_.clear(); }

  [[nodiscard]] bool empty() const { return intervals_.empty(); }

  [[nodiscard]] size_t size() const { return intervals_.size(); }

  [[nodiscard]] const_iterator begin() const { return intervals_.begin(); }

  [[nodiscard]] const_iterator end() const { return intervals_.end(); }

  [[nodiscard]] const_reverse_iterator rbegin() const { return intervals_.rbegin(); }

  [[nodiscard]] const_reverse_iterator rend() const { return intervals_.rend(); }

 private:
  // Intervals are disjoint and non-adjacent. The set keeps them sorted by start_inclusive.
  InternalSet intervals_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMESTAMP_INTERVAL_SET_H_
