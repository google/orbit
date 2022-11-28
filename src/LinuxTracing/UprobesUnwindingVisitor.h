// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_
#define LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <sys/types.h>

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "LeafFunctionCallManager.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "OrbitBase/Logging.h"
#include "PerfEvent.h"
#include "PerfEventRecords.h"
#include "PerfEventVisitor.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesReturnAddressManager.h"
#include "unwindstack/Unwinder.h"

namespace orbit_linux_tracing {

// UprobesUnwindingVisitor processes stack samples and entries/exits into dynamically instrumented
// functions (e.g., uprobes/uretprobes). It also processes memory maps changes, to keep necessary
// unwinding information up-to-date. It assumes all this information comes in order.
// The reason for processing samples and dynamic instrumentation in the same visitor is that, when
// entering a dynamically-instrumented function, the return address saved on the stack is hijacked
// (e.g., by uretprobes), so that the exit from the function can also be recorded.
// This causes unwinding of any (time-based) stack sample that falls inside such a function to stop
// at the first such function, with a frame in the trampoline (e.g., in the [uprobes] map).
// To solve this, UprobesReturnAddressManager keeps a stack, for every thread, of the return
// addresses before they are hijacked, and patches them into the time-based stack samples. Such
// return addresses can be retrieved by getting the eight bytes at the top of the stack when
// entering a dynamically instrumented function (e.g., when hitting uprobes).
class UprobesUnwindingVisitor : public PerfEventVisitor {
 public:
  explicit UprobesUnwindingVisitor(
      TracerListener* listener, UprobesFunctionCallManager* function_call_manager,
      UprobesReturnAddressManager* uprobes_return_address_manager, LibunwindstackMaps* initial_maps,
      LibunwindstackUnwinder* unwinder, LeafFunctionCallManager* leaf_function_call_manager,
      UserSpaceInstrumentationAddresses* user_space_instrumentation_addresses,
      const std::map<uint64_t, uint64_t>* absolute_address_to_size_of_functions_to_stop_at)
      : listener_{listener},
        function_call_manager_{function_call_manager},
        return_address_manager_{uprobes_return_address_manager},
        current_maps_{initial_maps},
        unwinder_{unwinder},
        leaf_function_call_manager_{leaf_function_call_manager},
        user_space_instrumentation_addresses_{user_space_instrumentation_addresses},
        absolute_address_to_size_of_functions_to_stop_at_{
            absolute_address_to_size_of_functions_to_stop_at} {
    ORBIT_CHECK(listener_ != nullptr);
    ORBIT_CHECK(function_call_manager_ != nullptr);
    ORBIT_CHECK(return_address_manager_ != nullptr);
    ORBIT_CHECK(current_maps_ != nullptr);
    ORBIT_CHECK(unwinder_ != nullptr);
    ORBIT_CHECK(leaf_function_call_manager_ != nullptr);
  }

  UprobesUnwindingVisitor(const UprobesUnwindingVisitor&) = delete;
  UprobesUnwindingVisitor& operator=(const UprobesUnwindingVisitor&) = delete;

  UprobesUnwindingVisitor(UprobesUnwindingVisitor&&) = default;
  UprobesUnwindingVisitor& operator=(UprobesUnwindingVisitor&&) = default;

  void SetUnwindErrorsAndDiscardedSamplesCounters(
      std::atomic<uint64_t>* unwind_error_counter,
      std::atomic<uint64_t>* samples_in_uretprobes_counter) {
    unwind_error_counter_ = unwind_error_counter;
    samples_in_uretprobes_counter_ = samples_in_uretprobes_counter;
  }

  void Visit(uint64_t event_timestamp, const StackSamplePerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const SchedWakeupWithCallchainPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const SchedSwitchWithCallchainPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const SchedWakeupWithStackPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const SchedSwitchWithStackPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const CallchainSamplePerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const UprobesPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const UprobesWithStackPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const UprobesWithArgumentsPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const UretprobesPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const UretprobesWithReturnValuePerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const UserSpaceFunctionEntryPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp,
             const UserSpaceFunctionExitPerfEventData& event_data) override;
  void Visit(uint64_t event_timestamp, const MmapPerfEventData& event_data) override;

 private:
  // This struct holds a copy of some stack data collected from the target process.
  struct StackSlice {
    uint64_t start_address;
    uint64_t size;
    std::unique_ptr<uint8_t[]> data;
  };

  void OnUprobes(uint64_t timestamp_ns, pid_t tid, uint32_t cpu, uint64_t sp, uint64_t ip,
                 uint64_t return_address,
                 std::optional<perf_event_sample_regs_user_sp_ip_arguments> registers,
                 uint64_t function_id);
  void OnUretprobes(uint64_t timestamp_ns, pid_t pid, pid_t tid, std::optional<uint64_t> ax);

  [[nodiscard]] orbit_grpc_protos::Callstack::CallstackType ComputeCallstackTypeFromStackSample(
      const LibunwindstackResult& libunwindstack_result);

  template <typename CallchainPerfEventDataT>
  [[nodiscard]] orbit_grpc_protos::Callstack::CallstackType
  ComputeCallstackTypeFromCallchainAndPatch(const CallchainPerfEventDataT& event_data);

  void SendFullAddressInfoToListener(const unwindstack::FrameData& libunwindstack_frame);

  template <typename StackPerfEventDataT>
  [[nodiscard]] bool UnwindStack(const StackPerfEventDataT& event,
                                 orbit_grpc_protos::Callstack* resulting_callstack,
                                 bool offline_memory_only = false);

  template <typename CallchainPerfEventDataT>
  [[nodiscard]] bool VisitCallchainEvent(const CallchainPerfEventDataT& event_data,
                                         orbit_grpc_protos::Callstack* resulting_callstack);

  TracerListener* listener_;

  UprobesFunctionCallManager* function_call_manager_;
  UprobesReturnAddressManager* return_address_manager_;
  LibunwindstackMaps* current_maps_;
  LibunwindstackUnwinder* unwinder_;
  LeafFunctionCallManager* leaf_function_call_manager_;

  UserSpaceInstrumentationAddresses* user_space_instrumentation_addresses_;

  const std::map<uint64_t, uint64_t>* absolute_address_to_size_of_functions_to_stop_at_;

  std::atomic<uint64_t>* unwind_error_counter_ = nullptr;
  std::atomic<uint64_t>* samples_in_uretprobes_counter_ = nullptr;

  absl::flat_hash_map<pid_t, std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>>
      uprobe_sps_ips_cpus_per_thread_{};
  absl::flat_hash_set<uint64_t> known_linux_address_infos_{};

  absl::flat_hash_map<pid_t, absl::flat_hash_map<uint64_t, StackSlice>>
      thread_id_stream_id_to_stack_slices_{};
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_
