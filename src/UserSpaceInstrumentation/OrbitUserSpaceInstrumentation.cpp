// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitUserSpaceInstrumentation.h"

#include <google/protobuf/arena.h>

#include <algorithm>
#include <stack>
#include <utility>
#include <variant>

#include "CaptureEventProducer/LockFreeBufferCaptureEventProducer.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Overloaded.h"
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

pid_t orbit_threads[] = {-1, -1, -1, -1, -1, -1};

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
        orbit_base::overloaded{[capture_event](const FunctionEntry& raw_event) -> void {
                                 orbit_grpc_protos::FunctionEntry* function_entry =
                                     capture_event->mutable_function_entry();
                                 function_entry->set_pid(raw_event.pid);
                                 function_entry->set_tid(raw_event.tid);
                                 function_entry->set_function_id(raw_event.function_id);
                                 function_entry->set_stack_pointer(raw_event.stack_pointer);
                                 function_entry->set_return_address(raw_event.return_address);
                                 function_entry->set_timestamp_ns(raw_event.timestamp_ns);
                               },
                               [capture_event](const FunctionExit& raw_event) -> void {
                                 orbit_grpc_protos::FunctionExit* function_exit =
                                     capture_event->mutable_function_exit();
                                 function_exit->set_pid(raw_event.pid);
                                 function_exit->set_tid(raw_event.tid);
                                 function_exit->set_timestamp_ns(raw_event.timestamp_ns);
                               }},
        raw_event);

    return capture_event;
  }

 private:
  template <class>
  [[maybe_unused]] static constexpr bool kAlwaysFalseV = false;
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

// NOTE: All symbols defined here have private linker visibility by default. Symbols that
// need to be visible to the tracee must be marked with `[[gnu::visibility("default")]]`. Check
// out the CMakeLists.txt file for more information.

// Initialize the LockFreeUserSpaceInstrumentationEventProducer and establish the connection to
// OrbitService.
[[gnu::visibility("default")]] void InitializeInstrumentation() { GetCaptureEventProducer(); }

[[gnu::visibility("default")]] void SetOrbitThreads(pid_t tid_0, pid_t tid_1, pid_t tid_2,
                                                    pid_t tid_3, pid_t tid_4, pid_t tid_5) {
  orbit_threads[0] = tid_0;
  orbit_threads[1] = tid_1;
  orbit_threads[2] = tid_2;
  orbit_threads[3] = tid_3;
  orbit_threads[4] = tid_4;
  orbit_threads[5] = tid_5;
}

[[gnu::visibility("default")]] void StartNewCapture(uint64_t capture_start_timestamp_ns) {
  current_capture_start_timestamp_ns = capture_start_timestamp_ns;
}

[[gnu::visibility("default")]] void EntryPayload(uint64_t return_address, uint64_t function_id,
                                                 uint64_t stack_pointer,
                                                 uint64_t return_trampoline_address) {
  bool& is_in_payload = GetIsInPayload();
  // If something in the callgraph below `EntryPayload` or `ExitPayload` was instrumented we need to
  // break the cycle here otherwise we would crash in an infinite recursion.
  if (is_in_payload) {
    return;
  }
  is_in_payload = true;

  thread_local const pid_t tid = orbit_base::GetCurrentThreadIdNative();

  if (tid == orbit_threads[0] || tid == orbit_threads[1] || tid == orbit_threads[2] ||
      tid == orbit_threads[3] || tid == orbit_threads[4] || tid == orbit_threads[5]) {
    is_in_payload = false;
    return;
  }

  const uint64_t timestamp_on_entry_ns = CaptureTimestampNs();

  std::stack<OpenFunctionCall>& open_function_call_stack = GetOpenFunctionCallStack();
  open_function_call_stack.emplace(return_address, timestamp_on_entry_ns);

  if (GetCaptureEventProducer().IsCapturing()) {
    static const uint32_t pid = orbit_base::GetCurrentProcessId();
    GetCaptureEventProducer().EnqueueIntermediateEvent(
        FunctionEntry{pid, orbit_base::FromNativeThreadId(tid), function_id, stack_pointer,
                      return_address, timestamp_on_entry_ns});
  }

  // Overwrite return address so that we end up returning to the exit trampoline.
  *reinterpret_cast<uint64_t*>(stack_pointer) = return_trampoline_address;

  is_in_payload = false;
}

[[gnu::visibility("default")]] uint64_t ExitPayload() {
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
