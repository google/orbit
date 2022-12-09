// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SCHEDULING_STATS_H_
#define ORBIT_GL_SCHEDULING_STATS_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/ThreadConstants.h"

class CaptureData;
class SchedulerTrack;

// SchedulingStats is a utility class to generate statistics about system-wide scheduling
// information. This includes core occupancy information as well as system-wide process and thread
// statistics.

class SchedulingStats {
 public:
  using ThreadNameProvider = std::function<std::string(int32_t)>;

  SchedulingStats() = delete;
  SchedulingStats(absl::Span<const orbit_client_protos::TimerInfo* const> scheduling_scopes,
                  const ThreadNameProvider& thread_name_provider, uint64_t start_ns,
                  uint64_t end_ns);

  [[nodiscard]] std::string ToString() const;

  struct ThreadStats {
    uint32_t tid = orbit_base::kInvalidThreadId;
    uint64_t time_on_core_ns = 0;
    std::string thread_name;
  };

  struct ProcessStats {
    std::map<int32_t, ThreadStats> thread_stats_by_tid;
    std::vector<ThreadStats*> thread_stats_sorted_by_time_on_core;
    uint32_t pid = orbit_base::kInvalidThreadId;
    uint64_t time_on_core_ns = 0;
    std::string process_name;
  };

  [[nodiscard]] double GetTimeRangeMs() const { return time_range_ms_; }
  [[nodiscard]] uint64_t GetTimeOnCoreNs() const { return time_on_core_ns_; }
  [[nodiscard]] const std::map<int32_t, uint64_t>& GetTimeOnCoreNsByCore() const {
    return time_on_core_ns_by_core_;
  }
  [[nodiscard]] const std::map<int32_t, ProcessStats>& GetProcessStatsByPid() const {
    return process_stats_by_pid_;
  }
  [[nodiscard]] const std::vector<ProcessStats*>& GetProcessStatsSortedByTimeOnCore() const {
    return process_stats_sorted_by_time_on_core_;
  }

 private:
  double time_range_ms_ = 0;
  uint64_t time_on_core_ns_ = 0;
  std::map<int32_t, uint64_t> time_on_core_ns_by_core_;
  std::map<int32_t, ProcessStats> process_stats_by_pid_;
  std::vector<ProcessStats*> process_stats_sorted_by_time_on_core_;
};

#endif  // ORBIT_GL_SCHEDULING_STATS_H_
