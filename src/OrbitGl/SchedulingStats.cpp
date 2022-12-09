// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/SchedulingStats.h"

#include <absl/strings/str_format.h>
#include <absl/types/span.h>

#include <algorithm>

#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Sort.h"

using orbit_client_protos::TimerInfo;

static constexpr double kNsToMs = 1 / 1000000.0;

SchedulingStats::SchedulingStats(absl::Span<const TimerInfo* const> scheduling_scopes,
                                 const ThreadNameProvider& thread_name_provider, uint64_t start_ns,
                                 uint64_t end_ns)
    : time_range_ms_(static_cast<double>(end_ns - start_ns) * kNsToMs) {
  // Iterate on every scope in the selected range to compute stats.
  for (const orbit_client_protos::TimerInfo* timer_info : scheduling_scopes) {
    uint64_t clipped_start_ns = std::max(start_ns, timer_info->start());
    uint64_t clipped_end_ns = std::min(end_ns, timer_info->end());
    uint64_t timer_duration_ns = clipped_end_ns - clipped_start_ns;

    time_on_core_ns_ += timer_duration_ns;
    time_on_core_ns_by_core_[timer_info->processor()] += timer_duration_ns;

    ProcessStats& process_stats = process_stats_by_pid_[timer_info->process_id()];
    process_stats.time_on_core_ns += timer_duration_ns;

    ThreadStats& thread_stats = process_stats.thread_stats_by_tid[timer_info->thread_id()];
    thread_stats.time_on_core_ns += timer_duration_ns;
  }

  // Iterate on every process and thread to finalize stats.
  for (auto& [pid, process_stats] : process_stats_by_pid_) {
    process_stats.process_name = thread_name_provider(pid);
    process_stats.pid = pid;
    process_stats_sorted_by_time_on_core_.push_back(&process_stats);
    for (auto& [tid, thread_stats] : process_stats.thread_stats_by_tid) {
      thread_stats.thread_name = thread_name_provider(tid);
      thread_stats.tid = tid;
      process_stats.thread_stats_sorted_by_time_on_core.push_back(&thread_stats);
    }
  }

  // Sort process stats by time on core.
  orbit_base::sort(process_stats_sorted_by_time_on_core_.begin(),
                   process_stats_sorted_by_time_on_core_.end(), &ProcessStats::time_on_core_ns,
                   std::greater<>{});

  // Sort thread stats by time on core.
  for (ProcessStats* process_stats : process_stats_sorted_by_time_on_core_) {
    orbit_base::sort(process_stats->thread_stats_sorted_by_time_on_core.begin(),
                     process_stats->thread_stats_sorted_by_time_on_core.end(),
                     &ThreadStats::time_on_core_ns, std::greater<>{});
  }
}

static double NsToMs(uint64_t ns) { return static_cast<double>(ns) * kNsToMs; }

std::string SchedulingStats::ToString() const {
  std::string summary;

  // Core occupancy.
  if (!time_on_core_ns_by_core_.empty()) summary += "Core occupancy: \n";
  for (const auto& [core, time_on_core_ns] : time_on_core_ns_by_core_) {
    double time_on_core_ms = static_cast<double>(time_on_core_ns) * kNsToMs;
    summary +=
        absl::StrFormat("cpu[%u] : %.2f%%\n", core, 100.0 * time_on_core_ms / time_range_ms_);
  }

  // Process and thread stats.
  if (time_range_ms_ > 0) summary += absl::StrFormat("\nSelection time: %.6f ms\n", time_range_ms_);
  for (ProcessStats* p_stats : process_stats_sorted_by_time_on_core_) {
    double p_time_on_core_ms = NsToMs(p_stats->time_on_core_ns);
    summary += absl::StrFormat("  %s[%i] spent %.6f ms on core (%.2f%%)\n", p_stats->process_name,
                               p_stats->pid, p_time_on_core_ms,
                               100.0 * p_time_on_core_ms / time_range_ms_);

    for (ThreadStats* t_stats : p_stats->thread_stats_sorted_by_time_on_core) {
      double t_time_on_core_ms = NsToMs(t_stats->time_on_core_ns);
      summary += absl::StrFormat("   - %s[%i] spent %.6f ms on core (%.2f%%)\n",
                                 t_stats->thread_name, t_stats->tid, t_time_on_core_ms,
                                 100.0 * t_time_on_core_ms / time_range_ms_);
    }
  }

  return summary;
}
