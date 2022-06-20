// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_THREAD_STATE_SLICE_INFO_H_
#define CLIENT_DATA_THREAD_STATE_SLICE_INFO_H_

#include <sys/types.h>

#include <cstdint>

#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

// This represents a time slice of a certain thread with a certain thread state.
// See `orbit_grpc_protos::ThreadStateSlice::ThreadState` for further information.
class ThreadStateSliceInfo {
 public:
  explicit ThreadStateSliceInfo(uint32_t tid, uint32_t was_blocked_by_thread,
                                uint32_t was_blocked_by_process,
                                orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state,
                                uint64_t begin_timestamp_ns, uint64_t end_timestamp_ns)
      : tid_{tid},
        was_blocked_by_thread_(was_blocked_by_thread),
        was_blocked_by_process_{was_blocked_by_process},
        thread_state_{thread_state},
        begin_timestamp_ns_{begin_timestamp_ns},
        end_timestamp_ns_{end_timestamp_ns} {}

  [[nodiscard]] uint32_t tid() const { return tid_; }
  [[nodiscard]] uint32_t was_blocked_by_thread() const { return was_blocked_by_thread_; }
  [[nodiscard]] uint32_t was_blocked_by_process() const { return was_blocked_by_process_; }
  [[nodiscard]] orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state() const {
    return thread_state_;
  }
  [[nodiscard]] uint64_t begin_timestamp_ns() const { return begin_timestamp_ns_; }
  [[nodiscard]] uint64_t end_timestamp_ns() const { return end_timestamp_ns_; }

 private:
  // pid is absent as we don't yet get that information from the service.
  uint32_t tid_;
  uint32_t was_blocked_by_thread_;
  uint32_t was_blocked_by_process_;
  orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state_;
  uint64_t begin_timestamp_ns_;
  uint64_t end_timestamp_ns_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_THREAD_STATE_SLICE_INFO_H_
