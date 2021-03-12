// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitCaptureClient/ApiEventProcessor.h"
#include "OrbitCaptureClient/CaptureEventProcessor.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "capture.pb.h"

using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::ApiEvent;
using orbit_grpc_protos::ClientCaptureEvent;

namespace {

// Helper empty implementation of CaptureListener to reduce the complexity of subclasses.
class EmptyCaptureListener : public CaptureListener {
 public:
  void OnCaptureStarted(ProcessData&&,
                        absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>,
                        TracepointInfoSet, absl::flat_hash_set<uint64_t>) override {}
  void OnTimer(const orbit_client_protos::TimerInfo&) override {}
  void OnKeyAndString(uint64_t, std::string) override {}
  void OnUniqueCallStack(CallStack) override {}
  void OnCallstackEvent(orbit_client_protos::CallstackEvent) override {}
  void OnThreadName(int32_t, std::string) override {}
  void OnThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo) override {}
  void OnAddressInfo(orbit_client_protos::LinuxAddressInfo) override {}
  void OnUniqueTracepointInfo(uint64_t, orbit_grpc_protos::TracepointInfo) override {}
  void OnTracepointEvent(orbit_client_protos::TracepointEventInfo) override {}
};

// Test CaptureListener used to validate TimerInfo data produced by api events.
class ApiEventCaptureListener : public EmptyCaptureListener {
 public:
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override {
    timers_.emplace_back(timer_info);
  }
  std::vector<TimerInfo> timers_;
};

// ApiTester exposes methods that mocks the Orbit API and internally creates grpc events that it
// forwards to both a CaptureEventProcessor and a ApiEventProcessor. Note that the
// CaptureEventProcessor contains a ApiEventProcessor. The test makes sure that both processors
// forward the same information to their listener. To help readability of the tests, the api methods
// return a ApiTester reference so that we chain function calls.
class ApiTester {
 public:
  ApiTester()
      : api_event_processor_(&api_event_listener_),
        capture_event_processor_(&capture_event_listener_) {}

  ApiTester& Start(const char* name = nullptr, orbit_api_color color = kOrbitColorAuto) {
    EnqueueApiEvent(orbit_api::kScopeStart, name, 0, color);
    return *this;
  }
  ApiTester& Stop() {
    EnqueueApiEvent(orbit_api::kScopeStop);
    return *this;
  }
  ApiTester& StartAsync(uint64_t id, const char* name = nullptr,
                        orbit_api_color color = kOrbitColorAuto) {
    EnqueueApiEvent(orbit_api::kScopeStartAsync, name, id, color);
    return *this;
  }
  ApiTester& StopAsync(uint64_t id) {
    EnqueueApiEvent(orbit_api::kScopeStopAsync, /*name=*/nullptr, id);
    return *this;
  }
  ApiTester& TrackValue(const char* name, uint64_t value, orbit_api_color color = kOrbitColorAuto) {
    EnqueueApiEvent(orbit_api::kTrackUint64, name, value, color);
    return *this;
  }

  ApiTester& CheckNumTimers(size_t num_timers) {
    EXPECT_EQ(api_event_listener_.timers_.size(), num_timers);
    EXPECT_EQ(capture_event_listener_.timers_.size(), num_timers);
    return *this;
  }

  size_t CountTimersOfType(orbit_api::EventType type) {
    const std::vector<TimerInfo>& timers = api_event_listener_.timers_;
    return std::count_if(timers.begin(), timers.end(), [type](const TimerInfo& timer_info) {
      return ApiEventFromTimerInfo(timer_info).type == type;
    });
  }

  ApiTester& CheckNumScopeTimers(size_t num_timers) {
    EXPECT_EQ(CountTimersOfType(orbit_api::kScopeStart), num_timers);
    return *this;
  }

  ApiTester& CheckNumAsyncScopeTimers(size_t num_timers) {
    EXPECT_EQ(CountTimersOfType(orbit_api::kScopeStartAsync), num_timers);
    return *this;
  }

  ApiTester& CheckNumTrackingTimers(size_t num_timers) {
    EXPECT_EQ(CountTimersOfType(orbit_api::kTrackUint64), num_timers);
    return *this;
  }

 private:
  void EnqueueApiEvent(orbit_api::EventType type, const char* name = nullptr, uint64_t data = 0,
                       orbit_api_color color = kOrbitColorAuto) {
    orbit_api::EncodedEvent encoded_event(type, name, data, color);
    ClientCaptureEvent client_capture_event;
    ApiEvent* api_event = client_capture_event.mutable_api_event();
    api_event->set_timestamp_ns(orbit_base::CaptureTimestampNs());
    api_event->set_pid(orbit_base::GetCurrentProcessId());
    api_event->set_tid(orbit_base::GetCurrentThreadId());
    api_event->set_r0(encoded_event.args[0]);
    api_event->set_r1(encoded_event.args[1]);
    api_event->set_r2(encoded_event.args[2]);
    api_event->set_r3(encoded_event.args[3]);
    api_event->set_r4(encoded_event.args[4]);
    api_event->set_r5(encoded_event.args[5]);

    api_event_processor_.ProcessApiEvent(*api_event);
    capture_event_processor_.ProcessEvent(client_capture_event);
  }

  [[nodiscard]] static orbit_api::Event ApiEventFromTimerInfo(
      const orbit_client_protos::TimerInfo& timer_info) {
    // On x64 Linux, 6 registers are used for integer argument passing.
    // Manual instrumentation uses those registers to encode orbit_api::Event
    // objects.
    constexpr size_t kNumIntegerRegisters = 6;
    CHECK(timer_info.registers_size() == kNumIntegerRegisters);
    uint64_t arg_0 = timer_info.registers(0);
    uint64_t arg_1 = timer_info.registers(1);
    uint64_t arg_2 = timer_info.registers(2);
    uint64_t arg_3 = timer_info.registers(3);
    uint64_t arg_4 = timer_info.registers(4);
    uint64_t arg_5 = timer_info.registers(5);
    orbit_api::EncodedEvent encoded_event(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    return encoded_event.event;
  }

  // ApiEventProcessor in isolation.
  ApiEventCaptureListener api_event_listener_;
  ApiEventProcessor api_event_processor_;

  // ApiEventProcessor as part of a CaptureEventProcessor.
  ApiEventCaptureListener capture_event_listener_;
  CaptureEventProcessor capture_event_processor_;
};

TEST(ApiEventProcessor, Scopes) {
  ApiTester api;
  api.Start("Scope0").CheckNumTimers(0);
  api.Start("Scope1").CheckNumTimers(0);
  api.Start("Scope2").CheckNumTimers(0);
  api.Stop().CheckNumTimers(1);
  api.Stop().CheckNumTimers(2);
  api.Stop().CheckNumTimers(3);

  api.CheckNumScopeTimers(3);
  api.CheckNumAsyncScopeTimers(0);
  api.CheckNumTrackingTimers(0);
}

TEST(ApiEventProcessor, AsyncScopes) {
  ApiTester api;
  api.StartAsync(0, "AsyncScope0").CheckNumTimers(0);
  api.StartAsync(1, "AsyncScope1").CheckNumTimers(0);
  api.StartAsync(2, "AsyncScope2").CheckNumTimers(0);
  api.StopAsync(1).CheckNumTimers(1);
  api.StopAsync(0).CheckNumTimers(2);
  api.StopAsync(2).CheckNumTimers(3);

  api.CheckNumScopeTimers(0);
  api.CheckNumAsyncScopeTimers(3);
  api.CheckNumTrackingTimers(0);
}

TEST(ApiEventProcessor, ValueTracking) {
  ApiTester api;
  api.TrackValue("ValueA", 0).CheckNumTimers(1);
  api.TrackValue("ValueA", 1).CheckNumTimers(2);
  api.TrackValue("ValueA", 2).CheckNumTimers(3);
  api.TrackValue("ValueA", 3).CheckNumTimers(4);

  api.CheckNumScopeTimers(0);
  api.CheckNumAsyncScopeTimers(0);
  api.CheckNumTrackingTimers(4);
}

TEST(ApiEventProcessor, ScopesWithMissingStart) {
  ApiTester api;
  api.Start("Scope1").CheckNumTimers(0);
  api.Start("Scope2").CheckNumTimers(0);
  api.Stop().CheckNumTimers(1);
  api.Stop().CheckNumTimers(2);
  api.Stop().CheckNumTimers(2);

  api.CheckNumScopeTimers(2);
  api.CheckNumAsyncScopeTimers(0);
  api.CheckNumTrackingTimers(0);
}

TEST(ApiEventProcessor, ScopesWithMissingStop) {
  ApiTester api;
  api.Start("Scope0").CheckNumTimers(0);
  api.Start("Scope1").CheckNumTimers(0);
  api.Start("Scope2").CheckNumTimers(0);
  api.Stop().CheckNumTimers(1);
  api.Stop().CheckNumTimers(2);

  api.CheckNumScopeTimers(2);
  api.CheckNumAsyncScopeTimers(0);
  api.CheckNumTrackingTimers(0);
}

TEST(ApiEventProcessor, MixedEvents) {
  ApiTester api;
  api.Start("Scope0").CheckNumTimers(0);
  api.StartAsync(0, "AsyncScope0").CheckNumTimers(0);
  api.TrackValue("ValueA", 0).CheckNumTimers(1).CheckNumTrackingTimers(1);
  api.TrackValue("ValueA", 0).CheckNumTimers(2).CheckNumTrackingTimers(2);
  api.Start("Scope1").CheckNumTimers(2).CheckNumScopeTimers(0);
  api.Start("Scope2").CheckNumTimers(2).CheckNumScopeTimers(0);
  api.Stop().CheckNumTimers(3).CheckNumScopeTimers(1);
  api.Stop().CheckNumTimers(4).CheckNumScopeTimers(2);
  api.TrackValue("ValueA", 0).CheckNumTimers(5).CheckNumTrackingTimers(3);
  api.StopAsync(0).CheckNumTimers(6).CheckNumAsyncScopeTimers(1);
  api.Stop().CheckNumTimers(7).CheckNumScopeTimers(3);
}

}  // namespace
