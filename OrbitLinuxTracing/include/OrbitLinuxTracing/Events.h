// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_EVENTS_H_
#define ORBIT_LINUX_TRACING_EVENTS_H_

#include <unistd.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace LinuxTracing {

class CallstackFrame {
 public:
  CallstackFrame(uint64_t pc, std::string function_name,
                 uint64_t function_offset, std::string map_name)
      : pc_(pc),
        function_name_(std::move(function_name)),
        function_offset_(function_offset),
        map_name_(std::move(map_name)) {}

  uint64_t GetPc() const { return pc_; }
  const std::string& GetFunctionName() const { return function_name_; }
  uint64_t GetFunctionOffset() const { return function_offset_; }
  const std::string& GetMapName() const { return map_name_; }
  static constexpr uint64_t kUnknownFunctionOffset =
      std::numeric_limits<uint64_t>::max();

 private:
  uint64_t pc_;
  std::string function_name_;
  uint64_t function_offset_;
  std::string map_name_;
};

class Callstack {
 public:
  Callstack(pid_t tid, std::vector<CallstackFrame> frames,
            uint64_t timestamp_ns)
      : tid_(tid), frames_(std::move(frames)), timestamp_ns_(timestamp_ns) {}

  pid_t GetTid() const { return tid_; }
  const std::vector<CallstackFrame>& GetFrames() const { return frames_; }
  uint64_t GetTimestampNs() const { return timestamp_ns_; }

 private:
  pid_t tid_;
  std::vector<CallstackFrame> frames_;
  uint64_t timestamp_ns_;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_EVENTS_H_
