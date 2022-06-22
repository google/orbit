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
  enum WakeupReason { kNotApplicable, kBlocked, kCreated };
  explicit ThreadStateSliceInfo(uint32_t tid,
                                orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state,
                                uint64_t begin_timestamp_ns, uint64_t end_timestamp_ns,
                                uint32_t wakeup_tid, uint32_t wakeup_pid,
                                WakeupReason wakeup_reason)
      : tid_{tid},
        wakeup_tid_(wakeup_tid),
        wakeup_pid_{wakeup_pid},
        wakeup_reason_{wakeup_reason},
        thread_state_{thread_state},
        begin_timestamp_ns_{begin_timestamp_ns},
        end_timestamp_ns_{end_timestamp_ns} {}

  [[nodiscard]] uint32_t tid() const { return tid_; }
  [[nodiscard]] uint32_t wakeup_tid() const { return wakeup_tid_; }
  [[nodiscard]] uint32_t wakeup_pid() const { return wakeup_pid_; }
  [[nodiscard]] WakeupReason wakeup_reason() const { return wakeup_reason_; }
  [[nodiscard]] orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state() const {
    return thread_state_;
  }
  [[nodiscard]] uint64_t begin_timestamp_ns() const { return begin_timestamp_ns_; }
  [[nodiscard]] uint64_t end_timestamp_ns() const { return end_timestamp_ns_; }

 private:
  // pid is absent as we don't yet get that information from the service.
  uint32_t tid_;
  uint32_t wakeup_tid_;
  uint32_t wakeup_pid_;
  WakeupReason wakeup_reason_;
  orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state_;
  uint64_t begin_timestamp_ns_;
  uint64_t end_timestamp_ns_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_THREAD_STATE_SLICE_INFO_H_
