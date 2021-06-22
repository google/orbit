// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/substitute.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/clock.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "GrpcProtos/Constants.h"
#include "MemoryTracing/MemoryInfoListener.h"
#include "MemoryTracing/MemoryInfoProducer.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"

namespace orbit_memory_tracing {
namespace {

using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_memory_tracing::MemoryInfoListener;
using orbit_memory_tracing::MemoryInfoProducer;

class BufferMemoryInfoListener : public MemoryInfoListener {
 public:
  void OnCGroupMemoryUsage(orbit_grpc_protos::CGroupMemoryUsage cgroup_memory_usage) override {
    ProducerCaptureEvent event;
    *event.mutable_cgroup_memory_usage() = std::move(cgroup_memory_usage);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnProcessMemoryUsage(orbit_grpc_protos::ProcessMemoryUsage process_memory_usage) override {
    ProducerCaptureEvent event;
    *event.mutable_process_memory_usage() = std::move(process_memory_usage);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  void OnSystemMemoryUsage(SystemMemoryUsage system_memory_usage) override {
    ProducerCaptureEvent event;
    *event.mutable_system_memory_usage() = std::move(system_memory_usage);
    {
      absl::MutexLock lock{&events_mutex_};
      events_.emplace_back(std::move(event));
    }
  }

  [[nodiscard]] std::vector<ProducerCaptureEvent> GetAndClearEvents() {
    absl::MutexLock lock{&events_mutex_};
    std::vector<ProducerCaptureEvent> events = std::move(events_);
    events_.clear();
    return events;
  }

 private:
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

    listener_.emplace();

    int32_t pid = static_cast<int32_t>(orbit_base::GetCurrentProcessId());
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

void VerifyOrderAndContentOfEvents(const std::vector<ProducerCaptureEvent>& events) {
  uint64_t previous_system_memory_event_timestamp_ns = 0;
  uint64_t previous_cgroup_memory_event_timestamp_ns = 0;
  uint64_t previous_process_memory_event_timestamp_ns = 0;
  for (const auto& event : events) {
    switch (event.event_case()) {
      case ProducerCaptureEvent::kSystemMemoryUsage: {
        // Verify whether events are in order of their timestamps.
        uint64_t current_system_memory_event_timestamp_ns =
            event.system_memory_usage().timestamp_ns();
        EXPECT_GE(current_system_memory_event_timestamp_ns,
                  previous_system_memory_event_timestamp_ns);

        // Verify whether the contents of events are valid.
        EXPECT_TRUE(event.system_memory_usage().total_kb() >= 0);
        EXPECT_TRUE(event.system_memory_usage().free_kb() >= 0);
        EXPECT_TRUE(event.system_memory_usage().available_kb() >= 0);
        EXPECT_TRUE(event.system_memory_usage().buffers_kb() >= 0);
        EXPECT_TRUE(event.system_memory_usage().cached_kb() >= 0);
        EXPECT_TRUE(event.system_memory_usage().pgfault() >= 0);
        EXPECT_TRUE(event.system_memory_usage().pgmajfault() >= 0);

        previous_system_memory_event_timestamp_ns = current_system_memory_event_timestamp_ns;
        break;
      }
      case ProducerCaptureEvent::kCgroupMemoryUsage: {
        // Verify whether events are in order of their timestamps.
        uint64_t current_cgroup_memory_event_timestamp_ns =
            event.cgroup_memory_usage().timestamp_ns();
        EXPECT_GE(current_cgroup_memory_event_timestamp_ns,
                  previous_cgroup_memory_event_timestamp_ns);

        // Verify whether the contents of events are valid.
        EXPECT_FALSE(event.cgroup_memory_usage().cgroup_name().empty());
        EXPECT_TRUE(event.cgroup_memory_usage().limit_bytes() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().rss_bytes() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().mapped_file_bytes() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().pgfault() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().pgmajfault() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().unevictable_bytes() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().inactive_anon_bytes() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().active_anon_bytes() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().inactive_file_bytes() >= 0);
        EXPECT_TRUE(event.cgroup_memory_usage().active_file_bytes() >= 0);

        previous_cgroup_memory_event_timestamp_ns = current_cgroup_memory_event_timestamp_ns;
        break;
      }
      case ProducerCaptureEvent::kProcessMemoryUsage: {
        // Verify whether events are in order of their timestamps.
        uint64_t current_process_memory_event_timestamp_ns =
            event.process_memory_usage().timestamp_ns();
        EXPECT_GE(current_process_memory_event_timestamp_ns,
                  previous_process_memory_event_timestamp_ns);

        // Verify whether the contents of events are valid.
        EXPECT_TRUE(event.process_memory_usage().rss_anon_kb() >= 0);
        EXPECT_TRUE(event.process_memory_usage().minflt() >= 0);
        EXPECT_TRUE(event.process_memory_usage().majflt() >= 0);

        previous_process_memory_event_timestamp_ns = current_process_memory_event_timestamp_ns;
        break;
      }
      default:
        UNREACHABLE();
    }
  }
}

// Verify whether the memory_sampling_period_ns_ works as expected by checking the number of
// received events.
void VerifyEventCounts(const std::vector<ProducerCaptureEvent>& events,
                       size_t num_expected_events_per_type) {
  constexpr size_t kEventCountsErrorTolerance = 2;

  size_t num_received_system_memory_events = 0;
  size_t num_received_cgroup_memory_events = 0;
  size_t num_received_process_memory_events = 0;
  for (const auto& event : events) {
    switch (event.event_case()) {
      case ProducerCaptureEvent::kSystemMemoryUsage: {
        num_received_system_memory_events++;
        break;
      }
      case ProducerCaptureEvent::kCgroupMemoryUsage: {
        num_received_cgroup_memory_events++;
        break;
      }
      case ProducerCaptureEvent::kProcessMemoryUsage: {
        num_received_process_memory_events++;
        break;
      }
      default:
        UNREACHABLE();
    }
  }

  EXPECT_GE(num_received_system_memory_events,
            num_expected_events_per_type - kEventCountsErrorTolerance);
  EXPECT_LE(num_received_system_memory_events,
            num_expected_events_per_type + kEventCountsErrorTolerance);

  EXPECT_GE(num_received_process_memory_events,
            num_expected_events_per_type - kEventCountsErrorTolerance);
  EXPECT_LE(num_received_process_memory_events,
            num_expected_events_per_type + kEventCountsErrorTolerance);

  // Verify the number of cgroup memory events only if the process's memory cgroup and the cgroup
  // memory.stat file can be found successfully
  if (num_received_cgroup_memory_events == 0) return;
  EXPECT_GE(num_received_cgroup_memory_events,
            num_expected_events_per_type - kEventCountsErrorTolerance);
  EXPECT_LE(num_received_cgroup_memory_events,
            num_expected_events_per_type + kEventCountsErrorTolerance);
}

}  // namespace

TEST(MemoryTracingIntegrationTest, MemoryTracing) {
  const uint64_t kMemorySamplingPeriodNs = absl::Milliseconds(100) / absl::Nanoseconds(1);
  const size_t kPeriodCounts = 10;

  MemoryTracingIntegrationTestFixture fixture(kMemorySamplingPeriodNs);

  absl::Duration tracing_period = absl::Nanoseconds(kMemorySamplingPeriodNs * kPeriodCounts);
  std::vector<ProducerCaptureEvent> events = TraceAndGetEvents(&fixture, tracing_period);

  VerifyOrderAndContentOfEvents(events);

  VerifyEventCounts(events, kPeriodCounts);
}

}  // namespace orbit_memory_tracing