// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/synchronization/mutex.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "MemoryTracing/MemoryInfoListener.h"
#include "MemoryTracing/MemoryInfoProducer.h"
#include "MemoryTracing/MemoryTracingUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_memory_tracing {
namespace {

using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::MemoryUsageEvent;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_memory_tracing::MemoryInfoListener;
using orbit_memory_tracing::MemoryInfoProducer;

class BufferMemoryInfoListener : public MemoryInfoListener {
 public:
  [[nodiscard]] std::vector<ProducerCaptureEvent> GetAndClearEvents() {
    absl::MutexLock lock{&events_mutex_};
    std::vector<ProducerCaptureEvent> events = std::move(events_);
    events_.clear();
    return events;
  }

 private:
  void OnMemoryUsageEvent(MemoryUsageEvent memory_usage_event) override {
    ProducerCaptureEvent event;
    *event.mutable_memory_usage_event() = std::move(memory_usage_event);

    absl::MutexLock lock{&events_mutex_};
    events_.emplace_back(std::move(event));
  }

  std::vector<ProducerCaptureEvent> events_;
  absl::Mutex events_mutex_;
};

class MemoryTracingIntegrationTestFixture {
 public:
  explicit MemoryTracingIntegrationTestFixture(uint64_t memory_sampling_period_ns)
      : memory_sampling_period_ns_(memory_sampling_period_ns) {}

  void StartTracing() {
    ORBIT_CHECK(!listener_.has_value());
    ORBIT_CHECK(system_memory_info_producer_ == nullptr);
    ORBIT_CHECK(cgroup_memory_info_producer_ == nullptr);
    ORBIT_CHECK(process_memory_info_producer_ == nullptr);

    listener_.emplace();
    listener_->SetSamplingStartTimestampNs(orbit_base::CaptureTimestampNs());
    listener_->SetSamplingPeriodNs(memory_sampling_period_ns_);
    // Collect the cgroup memory information only if the process's memory cgroup and the cgroup
    // memory.stat file can be found successfully
    pid_t pid = orbit_base::GetCurrentProcessIdNative();
    listener_->SetEnableCGroupMemory(GetCGroupMemoryUsage(pid).has_value());
    listener_->SetEnableProcessMemory(true);

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
    ORBIT_CHECK(listener_.has_value());
    ORBIT_CHECK(system_memory_info_producer_ != nullptr);
    ORBIT_CHECK(cgroup_memory_info_producer_ != nullptr);
    ORBIT_CHECK(process_memory_info_producer_ != nullptr);

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
  ORBIT_CHECK(fixture != nullptr);

  fixture->StartTracing();
  absl::SleepFor(tracing_period);
  return fixture->StopTracingAndGetEvents();
}

void VerifyOrderAndContentOfEvents(absl::Span<const ProducerCaptureEvent> events,
                                   uint64_t sampling_period_ns) {
  const auto memory_events_time_difference_tolerance =
      static_cast<uint64_t>(sampling_period_ns * 0.2);
  uint64_t previous_memory_usage_event_timestamp_ns = 0;

  for (const auto& event : events) {
    EXPECT_TRUE(event.event_case() == ProducerCaptureEvent::kMemoryUsageEvent);
    const MemoryUsageEvent& memory_usage_event = event.memory_usage_event();

    // Verify whether events are in order of their timestamps.
    uint64_t current_memory_usage_event_timestamp_ns = memory_usage_event.timestamp_ns();
    EXPECT_GE(current_memory_usage_event_timestamp_ns, previous_memory_usage_event_timestamp_ns);

    std::vector<uint64_t> timestamps;
    // Verify whether the contents of system memory events are valid.
    EXPECT_TRUE(memory_usage_event.has_system_memory_usage());
    const SystemMemoryUsage& system_memory_usage = memory_usage_event.system_memory_usage();
    timestamps.push_back(system_memory_usage.timestamp_ns());
    EXPECT_GE(system_memory_usage.total_kb(), 0);
    EXPECT_GE(system_memory_usage.free_kb(), 0);
    EXPECT_GE(system_memory_usage.available_kb(), 0);
    EXPECT_GE(system_memory_usage.buffers_kb(), 0);
    EXPECT_GE(system_memory_usage.cached_kb(), 0);
    EXPECT_GE(system_memory_usage.pgfault(), 0);
    EXPECT_GE(system_memory_usage.pgmajfault(), 0);

    // Verify whether the contents of process memory events are valid.
    EXPECT_TRUE(memory_usage_event.has_process_memory_usage());
    const ProcessMemoryUsage& process_memory_usage = memory_usage_event.process_memory_usage();
    timestamps.push_back(process_memory_usage.timestamp_ns());
    EXPECT_GE(process_memory_usage.rss_anon_kb(), 0);
    EXPECT_GE(process_memory_usage.minflt(), 0);
    EXPECT_GE(process_memory_usage.majflt(), 0);

    // If cgroup memory events are collected, verify whether the contents are valid.
    if (memory_usage_event.has_cgroup_memory_usage()) {
      const CGroupMemoryUsage& cgroup_memory_usage = memory_usage_event.cgroup_memory_usage();
      EXPECT_FALSE(cgroup_memory_usage.cgroup_name().empty());
      timestamps.push_back(cgroup_memory_usage.timestamp_ns());
      EXPECT_GE(cgroup_memory_usage.limit_bytes(), 0);
      EXPECT_GE(cgroup_memory_usage.rss_bytes(), 0);
      EXPECT_GE(cgroup_memory_usage.mapped_file_bytes(), 0);
      EXPECT_GE(cgroup_memory_usage.pgfault(), 0);
      EXPECT_GE(cgroup_memory_usage.pgmajfault(), 0);
      EXPECT_GE(cgroup_memory_usage.unevictable_bytes(), 0);
      EXPECT_GE(cgroup_memory_usage.inactive_anon_bytes(), 0);
      EXPECT_GE(cgroup_memory_usage.active_anon_bytes(), 0);
      EXPECT_GE(cgroup_memory_usage.inactive_file_bytes(), 0);
      EXPECT_GE(cgroup_memory_usage.active_file_bytes(), 0);
    }

    // Verify that the memory events in the same memory_usage_event are sampled at very close time.
    uint64_t max_timestamp = *std::max_element(timestamps.begin(), timestamps.end());
    uint64_t min_timestamp = *std::min_element(timestamps.begin(), timestamps.end());
    EXPECT_LE(max_timestamp - min_timestamp, memory_events_time_difference_tolerance);
  }
}

// Verify whether the memory_sampling_period_ns_ works as expected by checking the number of
// received events.
void VerifyEventCounts(absl::Span<const ProducerCaptureEvent> events, size_t expected_counts) {
  constexpr size_t kEventCountsErrorTolerance = 2;

  size_t num_received_events = 0;
  for (const auto& event : events) {
    if (event.event_case() == ProducerCaptureEvent::kMemoryUsageEvent) num_received_events++;
  }

  EXPECT_GE(num_received_events, expected_counts - kEventCountsErrorTolerance);
}

}  // namespace

TEST(MemoryTracingIntegrationTest, MemoryTracing) {
  const uint64_t memory_sampling_period_ns = absl::Milliseconds(100) / absl::Nanoseconds(1);
  const size_t period_counts = 10;

  MemoryTracingIntegrationTestFixture fixture(memory_sampling_period_ns);

  absl::Duration tracing_period = absl::Nanoseconds(memory_sampling_period_ns * period_counts);
  std::vector<ProducerCaptureEvent> events = TraceAndGetEvents(&fixture, tracing_period);

  VerifyOrderAndContentOfEvents(events, memory_sampling_period_ns);

  VerifyEventCounts(events, period_counts);
}

}  // namespace orbit_memory_tracing
