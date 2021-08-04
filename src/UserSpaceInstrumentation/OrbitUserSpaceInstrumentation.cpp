// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitUserSpaceInstrumentation.h"

#include <stdio.h>
#include <sys/types.h>

#include <cstdint>
#include <stack>

#include "CaptureEventProducer/LockFreeBufferCaptureEventProducer.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerSideChannel/ProducerSideChannel.h"

namespace {

using orbit_base::CaptureTimestampNs;

struct OpenFunctionCall {
  OpenFunctionCall(uint64_t return_address, uint64_t function_id, uint64_t timestamp_on_entry_ns)
      : return_address(return_address),
        function_id(function_id),
        timestamp_on_entry_ns(timestamp_on_entry_ns) {}
  uint64_t return_address;
  uint64_t function_id;
  uint64_t timestamp_on_entry_ns;
};

// The amount of data we store for each call is relevant for the overall performance. The assert is
// here for awareness and to avoid packing issues in the struct.
static_assert(sizeof(OpenFunctionCall) == 24, "OpenFunctionCall should be 24 bytes.");

std::stack<OpenFunctionCall>& GetOpenFunctionCallStack() {
  thread_local std::stack<OpenFunctionCall> open_function_calls;
  return open_function_calls;
}

uint64_t start_current_capture_timestamp = 0;

struct FunctionCallEvent {
  FunctionCallEvent() = default;
  FunctionCallEvent(int32_t pid, int32_t tid, uint64_t function_id, uint64_t duration_ns,
                    uint64_t end_timestamp_ns)
      : pid(pid),
        tid(tid),
        function_id(function_id),
        duration_ns(duration_ns),
        end_timestamp_ns(end_timestamp_ns) {}
  int32_t pid;
  int32_t tid;
  uint64_t function_id;
  uint64_t duration_ns;
  uint64_t end_timestamp_ns;
};

// The amount of data we transmit for each call is relevant for the overall performance. The assert
// is here for awareness and to avoid packing issues in the struct.
static_assert(sizeof(FunctionCallEvent) == 32, "FunctionCallEvent should be 32 bytes.");

// This class is used to enqueue FunctionCallEvent events from multiple threads and relay them to
// OrbitService in the form of orbit_grpc_protos::FunctionCall events.
class LockFreeUserSpaceInstrumentationEventProducer
    : public orbit_capture_event_producer::LockFreeBufferCaptureEventProducer<FunctionCallEvent> {
 public:
  LockFreeUserSpaceInstrumentationEventProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeUserSpaceInstrumentationEventProducer() { ShutdownAndWait(); }

 protected:
  [[nodiscard]] virtual orbit_grpc_protos::ProducerCaptureEvent* TranslateIntermediateEvent(
      FunctionCallEvent&& raw_event, google::protobuf::Arena* arena) override {
    auto* capture_event =
        google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);
    auto* function_call = capture_event->mutable_function_call();
    function_call->set_pid(raw_event.pid);
    function_call->set_tid(raw_event.tid);
    function_call->set_function_id(raw_event.function_id);
    function_call->set_duration_ns(raw_event.duration_ns);
    function_call->set_end_timestamp_ns(raw_event.end_timestamp_ns);
    return capture_event;
  }
};

}  // namespace

void StartNewCapture() { start_current_capture_timestamp = CaptureTimestampNs(); }

void EntryPayload(uint64_t return_address, uint64_t function_id) {
  const uint64_t timestamp_on_entry_ns = CaptureTimestampNs();
  auto& open_function_call_stack = GetOpenFunctionCallStack();
  open_function_call_stack.emplace(return_address, function_id, timestamp_on_entry_ns);
}

uint64_t ExitPayload() {
  const uint64_t timestamp_on_exit_ns = CaptureTimestampNs();
  auto& open_function_call_stack = GetOpenFunctionCallStack();
  OpenFunctionCall current_return_address = open_function_call_stack.top();
  open_function_call_stack.pop();

  static LockFreeUserSpaceInstrumentationEventProducer producer;
  // Skip emitting an event if we are not capturing or the event belongs to a previous capture.
  if (producer.IsCapturing() &&
      start_current_capture_timestamp < current_return_address.timestamp_on_entry_ns) {
    static pid_t pid = orbit_base::GetCurrentProcessId();
    thread_local pid_t tid = orbit_base::GetCurrentThreadId();
    const uint64_t duration_ns =
        timestamp_on_exit_ns - current_return_address.timestamp_on_entry_ns;
    producer.EnqueueIntermediateEvent(FunctionCallEvent(
        pid, tid, current_return_address.function_id, duration_ns, timestamp_on_exit_ns));
  }

  return current_return_address.return_address;
}
