// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitUserSpaceInstrumentation.h"

#include <sys/types.h>

#include <cstdint>
#include <stack>
#include <variant>

#include "CaptureEventProducer/LockFreeBufferCaptureEventProducer.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerSideChannel/ProducerSideChannel.h"

using orbit_base::CaptureTimestampNs;

namespace {

struct OpenFunctionCall {
  OpenFunctionCall(uint64_t return_address, uint64_t timestamp_on_entry_ns)
      : return_address(return_address), timestamp_on_entry_ns(timestamp_on_entry_ns) {}
  uint64_t return_address;
  uint64_t timestamp_on_entry_ns;
};

// The amount of data we store for each call is relevant for the overall performance. The assert is
// here for awareness and to avoid packing issues in the struct.
static_assert(sizeof(OpenFunctionCall) == 16, "OpenFunctionCall should be 16 bytes.");

std::stack<OpenFunctionCall>& GetOpenFunctionCallStack() {
  thread_local std::stack<OpenFunctionCall> open_function_calls;
  return open_function_calls;
}

uint64_t current_capture_start_timestamp_ns = 0;

// We can use the protos directly: since their fields are all integer fields, they are basically
// plain structs.
using FunctionEntryExitVariant =
    std::variant<orbit_grpc_protos::FunctionEntry, orbit_grpc_protos::FunctionExit>;

// This class is used to enqueue FunctionEntry and FunctionExit events from multiple threads and
// relay them to OrbitService.
class LockFreeUserSpaceInstrumentationEventProducer
    : public orbit_capture_event_producer::LockFreeBufferCaptureEventProducer<
          FunctionEntryExitVariant> {
 public:
  LockFreeUserSpaceInstrumentationEventProducer() {
    BuildAndStart(orbit_producer_side_channel::CreateProducerSideChannel());
  }

  ~LockFreeUserSpaceInstrumentationEventProducer() override { ShutdownAndWait(); }

 protected:
  [[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent* TranslateIntermediateEvent(
      FunctionEntryExitVariant&& raw_event, google::protobuf::Arena* arena) override {
    auto* capture_event =
        google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);

    std::visit(
        [capture_event](auto&& raw_event) {
          using DecayedEventType = std::decay_t<decltype(raw_event)>;
          if constexpr (std::is_same_v<DecayedEventType, orbit_grpc_protos::FunctionEntry>) {
            orbit_grpc_protos::FunctionEntry* function_entry =
                capture_event->mutable_function_entry();
            *function_entry = std::forward<decltype(raw_event)>(raw_event);
          } else if constexpr (std::is_same_v<DecayedEventType, orbit_grpc_protos::FunctionExit>) {
            orbit_grpc_protos::FunctionExit* function_exit = capture_event->mutable_function_exit();
            *function_exit = std::forward<decltype(raw_event)>(raw_event);
          } else {
            static_assert(always_false_v<DecayedEventType>, "Non-exhaustive visitor");
          }
        },
        std::move(raw_event));

    return capture_event;
  }

 private:
  template <class>
  [[maybe_unused]] static constexpr bool always_false_v = false;
};

LockFreeUserSpaceInstrumentationEventProducer& GetCaptureEventProducer() {
  static LockFreeUserSpaceInstrumentationEventProducer producer;
  return producer;
}

// Provide a thread local bool to keep track of whether the current thread is inside the payload we
// injected. If that is the case we avoid further instrumentation.
bool& GetIsInPayload() {
  thread_local bool is_in_payload = false;
  return is_in_payload;
}

}  // namespace

void StartNewCapture() {
  current_capture_start_timestamp_ns = CaptureTimestampNs();

  // If the library has just been injected, initialize the
  // LockFreeUserSpaceInstrumentationEventProducer and establish the connection to OrbitService now
  // instead of waiting for the first call to EntryPayload. As it takes a bit to
  // establish the connection, GetCaptureEventProducer().IsCapturing() would otherwise always be
  // false in the first call to EntryPayload, which would cause the first function call to be missed
  // even if the time between StartNewCapture() and the first function call is large.
  GetCaptureEventProducer();
}

void EntryPayload(uint64_t return_address, uint64_t function_id, uint64_t stack_pointer,
                  uint64_t return_trampoline_address) {
  bool& is_in_payload = GetIsInPayload();
  // If something in the callgraph below `EntryPayload` or `ExitPayload` was instrumented we need to
  // break the cycle here otherwise we would crash in an infinite recursion.
  if (is_in_payload) {
    return;
  }
  is_in_payload = true;

  const uint64_t timestamp_on_entry_ns = CaptureTimestampNs();

  std::stack<OpenFunctionCall>& open_function_call_stack = GetOpenFunctionCallStack();
  open_function_call_stack.emplace(return_address, timestamp_on_entry_ns);

  if (GetCaptureEventProducer().IsCapturing()) {
    static uint32_t pid = orbit_base::GetCurrentProcessId();
    thread_local uint32_t tid = orbit_base::GetCurrentThreadId();
    orbit_grpc_protos::FunctionEntry function_entry;
    function_entry.set_pid(pid);
    function_entry.set_tid(tid);
    function_entry.set_function_id(function_id);
    function_entry.set_stack_pointer(stack_pointer);
    function_entry.set_return_address(return_address);
    function_entry.set_timestamp_ns(timestamp_on_entry_ns);
    GetCaptureEventProducer().EnqueueIntermediateEvent(std::move(function_entry));
  }

  // Overwrite return address so that we end up returning to the exit trampoline.
  *reinterpret_cast<uint64_t*>(stack_pointer) = return_trampoline_address;

  is_in_payload = false;
}

uint64_t ExitPayload() {
  bool& is_in_payload = GetIsInPayload();
  is_in_payload = true;

  const uint64_t timestamp_on_exit_ns = CaptureTimestampNs();
  std::stack<OpenFunctionCall>& open_function_call_stack = GetOpenFunctionCallStack();
  OpenFunctionCall current_function_call = open_function_call_stack.top();
  open_function_call_stack.pop();

  // Skip emitting an event if we are not capturing or if the function call doesn't fully belong to
  // this capture.
  if (GetCaptureEventProducer().IsCapturing() &&
      current_capture_start_timestamp_ns < current_function_call.timestamp_on_entry_ns) {
    static uint32_t pid = orbit_base::GetCurrentProcessId();
    thread_local uint32_t tid = orbit_base::GetCurrentThreadId();
    orbit_grpc_protos::FunctionExit function_exit;
    function_exit.set_pid(pid);
    function_exit.set_tid(tid);
    function_exit.set_timestamp_ns(timestamp_on_exit_ns);
    GetCaptureEventProducer().EnqueueIntermediateEvent(std::move(function_exit));
  }

  is_in_payload = false;

  return current_function_call.return_address;
}
