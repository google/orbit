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
  enum class WakeupReason { kNotApplicable, kUnblocked, kCreated };
  explicit ThreadStateSliceInfo(uint32_t tid,
                                orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state,
                                uint64_t begin_timestamp_ns, uint64_t end_timestamp_ns,
                                WakeupReason wakeup_reason, uint32_t wakeup_tid,
                                uint32_t wakeup_pid,
                                std::optional<uint64_t> switch_out_or_wakeup_callstack_id)
      : tid_{tid},
        thread_state_{thread_state},
        begin_timestamp_ns_{begin_timestamp_ns},
        end_timestamp_ns_{end_timestamp_ns},
        wakeup_reason_{wakeup_reason},
        wakeup_tid_(wakeup_tid),
        wakeup_pid_{wakeup_pid},
        switch_out_or_wakeup_callstack_id_{switch_out_or_wakeup_callstack_id} {}

  [[nodiscard]] friend bool operator==(const ThreadStateSliceInfo& lhs,
                                       const ThreadStateSliceInfo& rhs) {
    return std::tie(lhs.tid_, lhs.thread_state_, lhs.begin_timestamp_ns_, lhs.end_timestamp_ns_,
                    lhs.wakeup_tid_, lhs.wakeup_pid_, lhs.wakeup_reason_,
                    lhs.switch_out_or_wakeup_callstack_id_) ==
           std::tie(rhs.tid_, rhs.thread_state_, rhs.begin_timestamp_ns_, rhs.end_timestamp_ns_,
                    rhs.wakeup_tid_, rhs.wakeup_pid_, rhs.wakeup_reason_,
                    rhs.switch_out_or_wakeup_callstack_id_);
  }
  [[nodiscard]] friend bool operator!=(const ThreadStateSliceInfo& lhs,
                                       const ThreadStateSliceInfo& rhs) {
    return !(lhs == rhs);
  }

  [[nodiscard]] uint32_t tid() const { return tid_; }
  [[nodiscard]] uint32_t wakeup_tid() const { return wakeup_tid_; }
  [[nodiscard]] uint32_t wakeup_pid() const { return wakeup_pid_; }
  [[nodiscard]] WakeupReason wakeup_reason() const { return wakeup_reason_; }
  [[nodiscard]] orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state() const {
    return thread_state_;
  }
  [[nodiscard]] uint64_t begin_timestamp_ns() const { return begin_timestamp_ns_; }
  [[nodiscard]] uint64_t end_timestamp_ns() const { return end_timestamp_ns_; }
  [[nodiscard]] std::optional<uint64_t> switch_out_or_wakeup_callstack_id() const {
    return switch_out_or_wakeup_callstack_id_;
  }

 private:
  // pid is absent as we don't yet get that information from the service.
  uint32_t tid_;
  orbit_grpc_protos::ThreadStateSlice::ThreadState thread_state_;
  uint64_t begin_timestamp_ns_;
  uint64_t end_timestamp_ns_;
  WakeupReason wakeup_reason_;
  uint32_t wakeup_tid_;
  uint32_t wakeup_pid_;
  std::optional<uint64_t> switch_out_or_wakeup_callstack_id_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_THREAD_STATE_SLICE_INFO_H_
