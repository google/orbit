// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/strings/substitute.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/clock.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <numeric>

#include "GrpcProtos/Constants.h"
#include "MemoryTracing/MemoryInfoListener.h"
#include "MemoryTracing/MemoryInfoProducer.h"
#include "MemoryTracingUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"

namespace orbit_memory_tracing {
namespace {

using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::MemoryEventWrapper;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_memory_tracing::MemoryInfoListener;
using orbit_memory_tracing::MemoryInfoProducer;

class BufferMemoryInfoListener : public MemoryInfoListener {
 public:
  explicit BufferMemoryInfoListener(uint64_t sampling_start_timestamp_ns,
                                    uint64_t sampling_period_ns, bool enable_cgroup_memory)
      : sampling_start_timestamp_ns_(sampling_start_timestamp_ns),
        sampling_period_ns_(sampling_period_ns),
        enable_cgroup_memory_(enable_cgroup_memory) {}

  void OnCGroupMemoryUsage(orbit_grpc_protos::CGroupMemoryUsage cgroup_memory_usage) override {
    uint64_t sampling_window_id = GetSamplingWindowId(cgroup_memory_usage.timestamp_ns());

    absl::MutexLock lock(&in_progress_wrappers_mutex_);
    *in_progress_wrappers_[sampling_window_id].mutable_cgroup_memory_usage() =
        std::move(cgroup_memory_usage);
    ProcessMemoryEventWrapperIfReady(sampling_window_id);
  }

  void OnProcessMemoryUsage(orbit_grpc_protos::ProcessMemoryUsage process_memory_usage) override {
    uint64_t sampling_window_id = GetSamplingWindowId(process_memory_usage.timestamp_ns());

    absl::MutexLock lock(&in_progress_wrappers_mutex_);
    *in_progress_wrappers_[sampling_window_id].mutable_process_memory_usage() =
        std::move(process_memory_usage);
    ProcessMemoryEventWrapperIfReady(sampling_window_id);
  }

  void OnSystemMemoryUsage(SystemMemoryUsage system_memory_usage) override {
    uint64_t sampling_window_id = GetSamplingWindowId(system_memory_usage.timestamp_ns());

    absl::MutexLock lock(&in_progress_wrappers_mutex_);
    *in_progress_wrappers_[sampling_window_id].mutable_system_memory_usage() =
        std::move(system_memory_usage);
    ProcessMemoryEventWrapperIfReady(sampling_window_id);
  }

  [[nodiscard]] std::vector<ProducerCaptureEvent> GetAndClearEvents() {
    absl::MutexLock lock{&events_mutex_};
    std::vector<ProducerCaptureEvent> events = std::move(events_);
    events_.clear();
    return events;
  }

 private:
  uint64_t GetSamplingWindowId(uint64_t sample_timestamp_ns) const {
    return static_cast<uint64_t>(
        std::round(static_cast<double>(sample_timestamp_ns - sampling_start_timestamp_ns_) /
                   sampling_period_ns_));
  }

  void ProcessMemoryEventWrapperIfReady(uint64_t sampling_window_id) {
    if (!in_progress_wrappers_.contains(sampling_window_id)) return;

    MemoryEventWrapper& wrapper = in_progress_wrappers_[sampling_window_id];
    bool wrapper_is_ready = wrapper.has_system_memory_usage() && wrapper.has_process_memory_usage();
    if (enable_cgroup_memory_) {
      wrapper_is_ready = wrapper_is_ready && wrapper.has_cgroup_memory_usage();
    }
    if (!wrapper_is_ready) return;

    std::vector<uint64_t> timestamps{wrapper.system_memory_usage().timestamp_ns(),
                                     wrapper.process_memory_usage().timestamp_ns()};
    if (enable_cgroup_memory_) timestamps.push_back(wrapper.cgroup_memory_usage().timestamp_ns());
    uint64_t synchronized_timestamp_ns = static_cast<uint64_t>(
        std::accumulate(timestamps.begin(), timestamps.end(), 0.0) / timestamps.size());
    wrapper.set_timestamp_ns(synchronized_timestamp_ns);

    ProducerCaptureEvent event;
    *event.mutable_memory_event_wrapper() = std::move(wrapper);
    in_progress_wrappers_.erase(sampling_window_id);

    absl::MutexLock lock{&events_mutex_};
    events_.emplace_back(std::move(event));
  }

  uint64_t sampling_start_timestamp_ns_;
  uint64_t sampling_period_ns_;
  bool enable_cgroup_memory_;

  absl::flat_hash_map<uint64_t, orbit_grpc_protos::MemoryEventWrapper> in_progress_wrappers_;
  absl::Mutex in_progress_wrappers_mutex_;

  std::vector<ProducerCaptureEvent> events_;
  absl::Mutex events_mutex_;
};

class MemoryTracingIntegrationTestFixture {
 public:
  explicit MemoryTracingIntegrationTestFixture(uint64_t memory_sampling_period_ns)
      : memory_sampling_period_ns_(memory_sampling_period_ns) {}

  void StartTracing() {
    CHECK(!listener_.has_value());
    CHECK(system_memory_info_producer_ == nullptr);
    CHECK(cgroup_memory_info_producer_ == nullptr);
    CHECK(process_memory_info_producer_ == nullptr);

    // Collect the cgroup memory information only if the process's memory cgroup and the cgroup
    // memory.stat file can be found successfully
    int32_t pid = static_cast<int32_t>(orbit_base::GetCurrentProcessId());
    bool enable_cgroup_memory = GetCGroupMemoryUsage(pid).has_value();

    listener_.emplace(orbit_base::CaptureTimestampNs(), memory_sampling_period_ns_,
                      enable_cgroup_memory);

    system_memory_info_producer_ =
        CreateSystemMemoryInfoProducer(&*listener_, memory_sampling_period_ns_, pid);
    system_memory_info_producer_->Start();

    cgroup_memory_info_producer_ =
        CreateCGroupMemoryInfoProducer(&*listener_, memory_sampling_period_ns_, pid);
    cgroup_memory_info_producer_->Start();

    process_memory_info_producer_ =
        CreateProcessMemoryInfoProducer(&*listener_, memory_sampling_period_ns_, pid);
    process_memory_info_producer_->Start();
  }

  [[nodiscard]] std::vector<ProducerCaptureEvent> StopTracingAndGetEvents() {
    CHECK(listener_.has_value());
    CHECK(system_memory_info_producer_ != nullptr);
    CHECK(cgroup_memory_info_producer_ != nullptr);
    CHECK(process_memory_info_producer_ != nullptr);

    system_memory_info_producer_->Stop();
    system_memory_info_producer_.reset();

    cgroup_memory_info_producer_->Stop();
    cgroup_memory_info_producer_.reset();

    process_memory_info_producer_->Stop();
    process_memory_info_producer_.reset();

    std::vector<ProducerCaptureEvent> events = listener_->GetAndClearEvents();
    listener_.reset();
    return events;
  }

 private:
  uint64_t memory_sampling_period_ns_;
  std::unique_ptr<MemoryInfoProducer> cgroup_memory_info_producer_;
  std::unique_ptr<MemoryInfoProducer> process_memory_info_producer_;
  std::unique_ptr<MemoryInfoProducer> system_memory_info_producer_;
  std::optional<BufferMemoryInfoListener> listener_ = std::nullopt;
};

[[nodiscard]] std::vector<ProducerCaptureEvent> TraceAndGetEvents(
    MemoryTracingIntegrationTestFixture* fixture, absl::Duration tracing_period) {
  CHECK(fixture != nullptr);

  fixture->StartTracing();
  absl::SleepFor(tracing_period);
  return fixture->StopTracingAndGetEvents();
}

void VerifyOrderAndContentOfEvents(const std::vector<ProducerCaptureEvent>& events,
                                   uint64_t sampling_period_ns) {
  const uint64_t kMemoryEventsTimeDifferenceTolerace =
      static_cast<uint64_t>(sampling_period_ns * 0.1);
  uint64_t previous_wrapper_timestamp_ns = 0;

  for (const auto& event : events) {
    EXPECT_TRUE(event.event_case() == ProducerCaptureEvent::kMemoryEventWrapper);
    const MemoryEventWrapper& wrapper = event.memory_event_wrapper();

    // Verify whether events are in order of their timestamps.
    uint64_t current_wrapper_timestamp_ns = wrapper.timestamp_ns();
    EXPECT_GE(current_wrapper_timestamp_ns, previous_wrapper_timestamp_ns);

    std::vector<uint64_t> timestamps;
    // Verify whether the contents of system memory events are valid.
    EXPECT_TRUE(wrapper.has_system_memory_usage());
    const SystemMemoryUsage& system_memory_usage = wrapper.system_memory_usage();
    timestamps.push_back(system_memory_usage.timestamp_ns());
    EXPECT_TRUE(system_memory_usage.total_kb() >= 0);
    EXPECT_TRUE(system_memory_usage.free_kb() >= 0);
    EXPECT_TRUE(system_memory_usage.available_kb() >= 0);
    EXPECT_TRUE(system_memory_usage.buffers_kb() >= 0);
    EXPECT_TRUE(system_memory_usage.cached_kb() >= 0);
    EXPECT_TRUE(system_memory_usage.pgfault() >= 0);
    EXPECT_TRUE(system_memory_usage.pgmajfault() >= 0);

    // Verify whether the contents of process memory events are valid.
    EXPECT_TRUE(wrapper.has_process_memory_usage());
    const ProcessMemoryUsage& process_memory_usage = wrapper.process_memory_usage();
    timestamps.push_back(process_memory_usage.timestamp_ns());
    EXPECT_TRUE(process_memory_usage.rss_anon_kb() >= 0);
    EXPECT_TRUE(process_memory_usage.minflt() >= 0);
    EXPECT_TRUE(process_memory_usage.majflt() >= 0);

    // If cgroup memory events are collected, verify whether the contents are valid.
    if (wrapper.has_cgroup_memory_usage()) {
      const CGroupMemoryUsage& cgroup_memory_usage = wrapper.cgroup_memory_usage();
      EXPECT_FALSE(cgroup_memory_usage.cgroup_name().empty());
      timestamps.push_back(cgroup_memory_usage.timestamp_ns());
      EXPECT_TRUE(cgroup_memory_usage.limit_bytes() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.rss_bytes() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.mapped_file_bytes() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.pgfault() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.pgmajfault() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.unevictable_bytes() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.inactive_anon_bytes() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.active_anon_bytes() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.inactive_file_bytes() >= 0);
      EXPECT_TRUE(cgroup_memory_usage.active_file_bytes() >= 0);
    }

    // Verify that the memory events in the same wrapper are sampled at very close time.
    uint64_t max_timestamp = *std::max_element(timestamps.begin(), timestamps.end());
    uint64_t min_timestamp = *std::min_element(timestamps.begin(), timestamps.end());
    EXPECT_LE(max_timestamp - min_timestamp, kMemoryEventsTimeDifferenceTolerace);
  }
}

// Verify whether the memory_sampling_period_ns_ works as expected by checking the number of
// received events.
void VerifyEventCounts(const std::vector<ProducerCaptureEvent>& events, size_t expected_counts) {
  constexpr size_t kEventCountsErrorTolerance = 2;

  size_t num_received_wrappers = 0;
  for (const auto& event : events) {
    if (event.event_case() == ProducerCaptureEvent::kMemoryEventWrapper) num_received_wrappers++;
  }

  EXPECT_GE(num_received_wrappers, expected_counts - kEventCountsErrorTolerance);
  EXPECT_LE(num_received_wrappers, expected_counts + kEventCountsErrorTolerance);
}

}  // namespace

TEST(MemoryTracingIntegrationTest, MemoryTracing) {
  const uint64_t kMemorySamplingPeriodNs = absl::Milliseconds(100) / absl::Nanoseconds(1);
  const size_t kPeriodCounts = 10;

  MemoryTracingIntegrationTestFixture fixture(kMemorySamplingPeriodNs);

  absl::Duration tracing_period = absl::Nanoseconds(kMemorySamplingPeriodNs * kPeriodCounts);
  std::vector<ProducerCaptureEvent> events = TraceAndGetEvents(&fixture, tracing_period);

  VerifyOrderAndContentOfEvents(events, kMemorySamplingPeriodNs);

  VerifyEventCounts(events, kPeriodCounts);
}

}  // namespace orbit_memory_tracing