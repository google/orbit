// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_EVENT_H_
#define CLIENT_DATA_CALLSTACK_EVENT_H_

#include <stdint.h>

namespace orbit_client_data {

// This class is used on the client to represent a callstack sample on a certain thread at a certain
// timestamp. The actual callstack is referenced by the callstack id.
class CallstackEvent {
 public:
  CallstackEvent() = delete;
  CallstackEvent(uint64_t timestamp_ns, uint64_t callstack_id, uint32_t thread_id)
      : timestamp_ns_{timestamp_ns}, callstack_id_{callstack_id}, thread_id_{thread_id} {}

  [[nodiscard]] uint64_t timestamp_ns() const { return timestamp_ns_; }
  [[nodiscard]] uint64_t callstack_id() const { return callstack_id_; }
  [[nodiscard]] uint32_t thread_id() const { return thread_id_; }

 private:
  uint64_t timestamp_ns_;
  uint64_t callstack_id_;
  uint32_t thread_id_;
};

inline bool operator==(const orbit_client_data::CallstackEvent& lhs,
                       const orbit_client_data::CallstackEvent& rhs) {
  return lhs.timestamp_ns() == rhs.timestamp_ns() && lhs.callstack_id() == rhs.callstack_id() &&
         lhs.thread_id() == rhs.thread_id();
}

template <typename H>
H AbslHashValue(H h, const orbit_client_data::CallstackEvent& o) {
  return H::combine(std::move(h), o.timestamp_ns(), o.callstack_id(), o.thread_id());
}

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CALLSTACK_EVENT_H_
