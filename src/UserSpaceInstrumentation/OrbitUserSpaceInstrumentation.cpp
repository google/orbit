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

// Don't use the orbit_grpc_protos::FunctionEntry and orbit_grpc_protos::FunctionExit protos
// directly. While in memory those protos are basically plain structs as their fields are all
// integer fields, their constructors and assignment operators are more complicated, and spend a lot
// of time in InternalSwap.
struct FunctionEntry {
  FunctionEntry() = default;
  FunctionEntry(uint32_t pid, uint32_t tid, uint64_t function_id, uint64_t stack_pointer,
                uint64_t return_address, uint64_t timestamp_ns)
      : pid{pid},
        tid{tid},
        function_id{function_id},
        stack_pointer{stack_pointer},
        return_address{return_address},
        timestamp_ns{timestamp_ns} {}
  uint32_t pid;
  uint32_t tid;
  uint64_t function_id;
  uint64_t stack_pointer;
  uint64_t return_address;
  uint64_t timestamp_ns;
};

struct FunctionExit {
  FunctionExit() = default;
  FunctionExit(uint32_t pid, uint32_t tid, uint64_t timestamp_ns)
      : pid{pid}, tid{tid}, timestamp_ns{timestamp_ns} {}
  uint32_t pid;
  uint32_t tid;
  uint64_t timestamp_ns;
};

using FunctionEntryExitVariant = std::variant<FunctionEntry, FunctionExit>;

// This class is used to enqueue FunctionEntry and FunctionExit events from multiple threads,
// transform them into orbit_grpc_protos::FunctionEntry and orbit_grpc_protos::FunctionExit protos,
// and relay them to OrbitService.
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
          if constexpr (std::is_same_v<DecayedEventType, FunctionEntry>) {
            orbit_grpc_protos::FunctionEntry* function_entry =
                capture_event->mutable_function_entry();
            function_entry->set_pid(raw_event.pid);
            function_entry->set_tid(raw_event.tid);
            function_entry->set_function_id(raw_event.function_id);
            function_entry->set_stack_pointer(raw_event.stack_pointer);
            function_entry->set_return_address(raw_event.return_address);
            function_entry->set_timestamp_ns(raw_event.timestamp_ns);
          } else if constexpr (std::is_same_v<DecayedEventType, FunctionExit>) {
            orbit_grpc_protos::FunctionExit* function_exit = capture_event->mutable_function_exit();
            function_exit->set_pid(raw_event.pid);
            function_exit->set_tid(raw_event.tid);
            function_exit->set_timestamp_ns(raw_event.timestamp_ns);
          } else {
            static_assert(always_false_v<DecayedEventType>, "Non-exhaustive visitor");
          }
        },
        raw_event);

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
  // TODO(b/205939288): The fix involving calling GetCaptureEventProducer() here was removed because
  //  of b/209560448 (we could have interrupted a malloc, which is not re-entrant, so we need to
  //  avoid any memory allocation). Re-add the call once we have a solution to allow re-entrancy.
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
    GetCaptureEventProducer().EnqueueIntermediateEvent(
        FunctionEntry{pid, tid, function_id, stack_pointer, return_address, timestamp_on_entry_ns});
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
    GetCaptureEventProducer().EnqueueIntermediateEvent(
        FunctionExit{pid, tid, timestamp_on_exit_ns});
  }

  is_in_payload = false;

  return current_function_call.return_address;
}
