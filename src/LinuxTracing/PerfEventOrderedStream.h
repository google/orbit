// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_PERF_EVENT_ORDERED_STREAM_H_
#define LINUX_TRACING_PERF_EVENT_ORDERED_STREAM_H_

#include <absl/hash/hash.h>
#include <stdint.h>
#include <sys/types.h>

namespace orbit_linux_tracing {

// This class holds information on streams on which `PerfEvent`s can be assumed in relative order of
// timestamp. The information is used by `PerfEventQueue`.
// In addition to the lack of order, the supported ordered streams are perf_event_open ring buffers
// (identified by file descriptor) and threads (identified by thread id).
class PerfEventOrderedStream {
 public:
  static const PerfEventOrderedStream kNone;

  static PerfEventOrderedStream FileDescriptor(int fd) {
    return PerfEventOrderedStream{OrderType::kOrderedInFileDescriptor, fd};
  }

  static PerfEventOrderedStream ThreadId(pid_t tid) {
    return PerfEventOrderedStream{OrderType::kOrderedInThreadId, tid};
  }

  // Make this class hashable for use as key in an absl::flat_hash_map.
  template <typename H>
  friend H AbslHashValue(H state, const PerfEventOrderedStream& order) {
    return H::combine(std::move(state), order.order_type_, order.order_value_);
  }

  friend bool operator==(const PerfEventOrderedStream& lhs, const PerfEventOrderedStream& rhs) {
    return lhs.order_type_ == rhs.order_type_ && lhs.order_value_ == rhs.order_value_;
  }

  friend bool operator!=(const PerfEventOrderedStream& lhs, const PerfEventOrderedStream& rhs) {
    return !(lhs == rhs);
  }

 private:
  enum class OrderType {
    kNotOrdered = 0,
    kOrderedInFileDescriptor,
    kOrderedInThreadId,
  };

  // The internal representation need not and should not be externally accessible. From the outside,
  // the only information that should be used is whether two instances of this class indicate the
  // same stream of ordered events.
  OrderType order_type_;
  // Conveniently, both file descriptors and thread ids are signed 32-bit integers.
  int32_t order_value_;

  PerfEventOrderedStream(OrderType order_type, int32_t order_value)
      : order_type_{order_type}, order_value_{order_value} {}
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_PERF_EVENT_ORDERED_STREAM_H_
