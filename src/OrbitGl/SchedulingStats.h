// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SCHEDULING_STATS_H_
#define ORBIT_GL_SCHEDULING_STATS_H_

#include <functional>
#include <map>
#include <string>
#include <vector>

class CaptureData;
class SchedulerTrack;
class TextBox;

// SchedulingStats is a utility class to generate statistics about system-wide scheduling
// information. This includes core occupancy information as well as system-wide process and thread
// statistics.

class SchedulingStats {
 public:
  typedef std::function<std::string(int32_t)> ThreadNameProvider;

  SchedulingStats() = delete;
  SchedulingStats(const std::vector<const TextBox*>& scheduling_scopes,
                  const ThreadNameProvider& thread_name_provider, uint64_t start_ns,
                  uint64_t end_ns);

  [[nodiscard]] std::string ToString() const;

  struct ThreadStats {
    int32_t tid = -1;
    uint64_t time_on_core_ns = 0;
    std::string thread_name;
  };

  struct ProcessStats {
    std::map<int32_t, ThreadStats> thread_stats_by_tid;
    std::vector<ThreadStats*> thread_stats_sorted_by_time_on_core;
    int32_t pid = -1;
    uint64_t time_on_core_ns = 0;
    std::string process_name;
  };

  struct ProcessStatsTimeSorter {
    bool operator()(const ProcessStats* a, const ProcessStats* b) const {
      return a->time_on_core_ns > b->time_on_core_ns;
    }
  };

  struct ThreadStatsTimeSorter {
    bool operator()(const ThreadStats* a, const ThreadStats* b) const {
      return a->time_on_core_ns > b->time_on_core_ns;
    }
  };

  double GetTimeRangeMs() const { return time_range_ms_; }
  uint64_t GetTimeOnCoreNs() const { return time_on_core_ns_; }
  const std::map<int32_t, uint64_t>& GetTimeOnCoreNsByCore() const {
    return time_on_core_ns_by_core_;
  }
  const std::map<int32_t, ProcessStats>& GetProcessStatsByPid() const {
    return process_stats_by_pid_;
  }
  const std::vector<ProcessStats*>& GetProcessStatsSortedByTimeOnCore() const {
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
