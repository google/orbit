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
#include "MemoryTracing/SystemMemoryInfoProducer.h"
#include "OrbitBase/Logging.h"
#include "capture.pb.h"

namespace orbit_memory_tracing {
namespace {

using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_memory_tracing::MemoryInfoListener;
using orbit_memory_tracing::SystemMemoryInfoProducer;

class BufferMemoryInfoListener : public MemoryInfoListener {
 public:
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

  void OnProcessMemoryUsage(orbit_grpc_protos::ProcessMemoryUsage /*memory_info*/) override {}
  void OnCGroupMemoryUsage(orbit_grpc_protos::CGroupMemoryUsage /*memory_info*/) override {}

 private:
  std::vector<ProducerCaptureEvent> events_;
  absl::Mutex events_mutex_;
};

class MemoryTracingIntegrationTestFixture {
 public:
  explicit MemoryTracingIntegrationTestFixture(uint64_t memory_sampling_period_ns)
      : memory_sampling_period_ns_(memory_sampling_period_ns) {}

  void StartTracing() {
    CHECK(!producer_.has_value());
    CHECK(!listener_.has_value());
    producer_.emplace(memory_sampling_period_ns_);
    listener_.emplace();
    producer_->SetListener(&*listener_);
    producer_->Start();
  }

  [[nodiscard]] std::vector<ProducerCaptureEvent> StopTracingAndGetEvents() {
    CHECK(producer_.has_value());
    CHECK(listener_.has_value());
    producer_->Stop();
    producer_.reset();
    std::vector<ProducerCaptureEvent> events = listener_->GetAndClearEvents();
    listener_.reset();
    return events;
  }

 private:
  uint64_t memory_sampling_period_ns_;
  std::optional<SystemMemoryInfoProducer> producer_ = std::nullopt;
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
  uint64_t previous_event_timestamp_ns = 0;
  for (const auto& event : events) {
    CHECK(event.event_case() == ProducerCaptureEvent::kSystemMemoryUsage);

    // Verify whether events are in order of their timestamps.
    uint64_t current_event_timestamp_ns = event.system_memory_usage().timestamp_ns();
    EXPECT_GE(current_event_timestamp_ns, previous_event_timestamp_ns);

    // Verify whether the contents of events are valid.
    EXPECT_TRUE(event.system_memory_usage().total_kb() > 0);
    EXPECT_TRUE(event.system_memory_usage().free_kb() > 0);
    EXPECT_TRUE(event.system_memory_usage().available_kb() > 0);
    EXPECT_TRUE(event.system_memory_usage().buffers_kb() > 0);
    EXPECT_TRUE(event.system_memory_usage().cached_kb() > 0);

    previous_event_timestamp_ns = current_event_timestamp_ns;
  }
}

// Verify whether the memory_sampling_period_ns_ works as expected by checking the number of
// received events.
void VerifyEventCounts(const std::vector<ProducerCaptureEvent>& events,
                       size_t expected_event_counts) {
  constexpr size_t kEventCountsErrorTolerance = 2;

  size_t received_event_counts = events.size();
  EXPECT_GE(received_event_counts, expected_event_counts - kEventCountsErrorTolerance);
  EXPECT_LE(received_event_counts, expected_event_counts + kEventCountsErrorTolerance);
}

}  // namespace

TEST(MemoryTracingIntegrationTest, SystemMemoryUsageTracing) {
  const uint64_t kMemorySamplingPeriodNs = absl::Milliseconds(100) / absl::Nanoseconds(1);
  const size_t kExpectedEventCounts = 10;

  MemoryTracingIntegrationTestFixture fixture(kMemorySamplingPeriodNs);

  absl::Duration tracing_period = absl::Nanoseconds(kMemorySamplingPeriodNs * kExpectedEventCounts);
  std::vector<ProducerCaptureEvent> events = TraceAndGetEvents(&fixture, tracing_period);

  VerifyOrderAndContentOfEvents(events);

  VerifyEventCounts(events, kExpectedEventCounts);
}

}  // namespace orbit_memory_tracing