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

// Empty implementation of CaptureListener used as helper to reduce the complexity in subclasses.
class EmptyCaptureListener : public CaptureListener {
 public:
  void OnCaptureStarted(ProcessData&&,
                        absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction>,
                        TracepointInfoSet, absl::flat_hash_set<uint64_t>) override {}
  void OnTimer(const orbit_client_protos::TimerInfo&) override {}
  void OnSystemMemoryUsage(const orbit_grpc_protos::SystemMemoryUsage&) override {}
  void OnKeyAndString(uint64_t, std::string) override {}
  void OnUniqueCallStack(CallStack) override {}
  void OnCallstackEvent(orbit_client_protos::CallstackEvent) override {}
  void OnThreadName(int32_t, std::string) override {}
  void OnModuleUpdate(uint64_t /*timestamp_ns*/,
                      orbit_grpc_protos::ModuleInfo /*module_info*/) override {}
  void OnModulesSnapshot(uint64_t /*timestamp_ns*/,
                         std::vector<orbit_grpc_protos::ModuleInfo> /*module_infos*/) override {}
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
// forwards to both a CaptureEventProcessor and an ApiEventProcessor. The test makes sure that both
// processors forward the same information to their listener. Note that a CaptureEventProcessor owns
// an ApiEventProcessor to which it forwards ApiEvent events. To help readability of test code, the
// api methods return an ApiTester reference so that we can chain function calls.
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

  ApiTester& ExpectNumTimers(size_t num_timers) {
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

  ApiTester& ExpectNumScopeTimers(size_t num_timers) {
    EXPECT_EQ(CountTimersOfType(orbit_api::kScopeStart), num_timers);
    return *this;
  }

  ApiTester& ExpectNumAsyncScopeTimers(size_t num_timers) {
    EXPECT_EQ(CountTimersOfType(orbit_api::kScopeStartAsync), num_timers);
    return *this;
  }

  ApiTester& ExpectNumTrackingTimers(size_t num_timers) {
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
  api.Start("Scope0").ExpectNumTimers(0);
  api.Start("Scope1").ExpectNumTimers(0);
  api.Start("Scope2").ExpectNumTimers(0);
  api.Stop().ExpectNumTimers(1);
  api.Stop().ExpectNumTimers(2);
  api.Stop().ExpectNumTimers(3);

  api.ExpectNumScopeTimers(3);
  api.ExpectNumAsyncScopeTimers(0);
  api.ExpectNumTrackingTimers(0);
}

TEST(ApiEventProcessor, AsyncScopes) {
  ApiTester api;
  api.StartAsync(0, "AsyncScope0").ExpectNumTimers(0);
  api.StartAsync(1, "AsyncScope1").ExpectNumTimers(0);
  api.StartAsync(2, "AsyncScope2").ExpectNumTimers(0);
  api.StopAsync(1).ExpectNumTimers(1);
  api.StopAsync(0).ExpectNumTimers(2);
  api.StopAsync(2).ExpectNumTimers(3);

  api.ExpectNumScopeTimers(0);
  api.ExpectNumAsyncScopeTimers(3);
  api.ExpectNumTrackingTimers(0);
}

TEST(ApiEventProcessor, ValueTracking) {
  ApiTester api;
  api.TrackValue("ValueA", 0).ExpectNumTimers(1);
  api.TrackValue("ValueA", 1).ExpectNumTimers(2);
  api.TrackValue("ValueA", 2).ExpectNumTimers(3);
  api.TrackValue("ValueA", 3).ExpectNumTimers(4);

  api.ExpectNumScopeTimers(0);
  api.ExpectNumAsyncScopeTimers(0);
  api.ExpectNumTrackingTimers(4);
}

TEST(ApiEventProcessor, ScopesWithMissingStart) {
  ApiTester api;
  api.Start("Scope1").ExpectNumTimers(0);
  api.Start("Scope2").ExpectNumTimers(0);
  api.Stop().ExpectNumTimers(1);
  api.Stop().ExpectNumTimers(2);
  api.Stop().ExpectNumTimers(2);

  api.ExpectNumScopeTimers(2);
  api.ExpectNumAsyncScopeTimers(0);
  api.ExpectNumTrackingTimers(0);
}

TEST(ApiEventProcessor, ScopesWithMissingStop) {
  ApiTester api;
  api.Start("Scope0").ExpectNumTimers(0);
  api.Start("Scope1").ExpectNumTimers(0);
  api.Start("Scope2").ExpectNumTimers(0);
  api.Stop().ExpectNumTimers(1);
  api.Stop().ExpectNumTimers(2);

  api.ExpectNumScopeTimers(2);
  api.ExpectNumAsyncScopeTimers(0);
  api.ExpectNumTrackingTimers(0);
}

TEST(ApiEventProcessor, MixedEvents) {
  ApiTester api;
  api.Start("Scope0").ExpectNumTimers(0);
  api.StartAsync(0, "AsyncScope0").ExpectNumTimers(0);
  api.TrackValue("ValueA", 0).ExpectNumTimers(1).ExpectNumTrackingTimers(1);
  api.TrackValue("ValueA", 0).ExpectNumTimers(2).ExpectNumTrackingTimers(2);
  api.Start("Scope1").ExpectNumTimers(2).ExpectNumScopeTimers(0);
  api.Start("Scope2").ExpectNumTimers(2).ExpectNumScopeTimers(0);
  api.Stop().ExpectNumTimers(3).ExpectNumScopeTimers(1);
  api.Stop().ExpectNumTimers(4).ExpectNumScopeTimers(2);
  api.TrackValue("ValueA", 0).ExpectNumTimers(5).ExpectNumTrackingTimers(3);
  api.StopAsync(0).ExpectNumTimers(6).ExpectNumAsyncScopeTimers(1);
  api.Stop().ExpectNumTimers(7).ExpectNumScopeTimers(3);
}

}  // namespace
