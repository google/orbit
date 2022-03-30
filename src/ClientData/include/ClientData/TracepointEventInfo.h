// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TRACEPOINT_EVENT_INFO_H_
#define CLIENT_DATA_TRACEPOINT_EVENT_INFO_H_

#include <cstdint>

namespace orbit_client_data {

// This class is used on the client to represent a tracepoint event sample on a certain thread at a
// certain timestamp. The actual tracepoint is referenced by the tracepoint id.
// See `orbit_grpc_protos::TracePointEvent`.
class TracepointEventInfo {
 public:
  TracepointEventInfo() = delete;
  TracepointEventInfo(uint32_t pid, uint32_t tid, int32_t cpu, uint64_t timestamp_ns,
                      uint64_t tracepoint_id)
      : pid_(pid),
        tid_(tid),
        cpu_(cpu),
        timestamp_ns_(timestamp_ns),
        tracepoint_id_(tracepoint_id) {}

  [[nodiscard]] uint32_t pid() const { return pid_; }
  [[nodiscard]] uint32_t tid() const { return tid_; }
  [[nodiscard]] int32_t cpu() const { return cpu_; }
  [[nodiscard]] uint64_t timestamp_ns() const { return timestamp_ns_; }
  [[nodiscard]] uint64_t tracepoint_id() const { return tracepoint_id_; }

 private:
  uint32_t pid_;
  uint32_t tid_;
  int32_t cpu_;
  uint64_t timestamp_ns_;
  uint64_t tracepoint_id_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TRACEPOINT_EVENT_INFO_H_
