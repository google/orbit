// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UprobesUnwindingVisitor.h"

#include <absl/types/span.h>
#include <sys/mman.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Object.h>
#include <unwindstack/PeCoff.h>
#include <unwindstack/SharedString.h>
#include <unwindstack/Unwinder.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "LibunwindstackMultipleOfflineAndProcessMemory.h"
#include "ModuleUtils/ReadLinuxModules.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "PerfEvent.h"
#include "unwindstack/Arch.h"
#include "unwindstack/Maps.h"
#include "unwindstack/Memory.h"

namespace orbit_linux_tracing {

using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::FullAddressInfo;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::ThreadStateSliceCallstack;

static bool CallstackIsInUserSpaceInstrumentation(
    absl::Span<const unwindstack::FrameData> frames,
    const UserSpaceInstrumentationAddresses& user_space_instrumentation_addresses) {
  ORBIT_CHECK(!frames.empty());

  // This case is for a sample falling directly inside a user space instrumentation trampoline.
  if (user_space_instrumentation_addresses.IsInEntryOrReturnTrampoline(frames.front().pc)) {
    return true;
  }

  // This case is for all samples falling in a callee of the trampoline. These are normally in the
  // injected library, but they could also be in a module containing a function called by the
  // library. So we check if *any* frame is in the injected library. If one is found, we then check
  // if any of the previous frames corresponds to a trampoline.
  std::string_view injected_library_map_name =
      user_space_instrumentation_addresses.GetInjectedLibraryMapName();
  auto library_frame_it = std::find_if(
      frames.begin(), frames.end(),
      [&injected_library_map_name](const unwindstack::FrameData& frame) {
        return frame.map_info != nullptr && frame.map_info->name() == injected_library_map_name;
      });
  if (library_frame_it == frames.end()) {
    return false;
  }
  return std::any_of(
      library_frame_it + 1, frames.end(),
      [&user_space_instrumentation_addresses](const unwindstack::FrameData& frame) {
        return user_space_instrumentation_addresses.IsInEntryOrReturnTrampoline(frame.pc);
      });
}

static bool CallchainIsInUserSpaceInstrumentation(
    const uint64_t* callchain, uint64_t callchain_size, LibunwindstackMaps& maps,
    const UserSpaceInstrumentationAddresses& user_space_instrumentation_addresses) {
  ORBIT_CHECK(callchain_size >= 2);

  // This case is for a sample falling directly inside a user space instrumentation trampoline.
  if (user_space_instrumentation_addresses.IsInEntryOrReturnTrampoline(callchain[1])) {
    return true;
  }

  // This case is for all samples falling in a callee of the trampoline. These are normally in the
  // injected library, but they could also be in a module containing a function called by the
  // library. So we check if *any* frame is in the injected library. If one is found, we then check
  // if any of the previous frames corresponds to a trampoline.
  std::string_view injected_library_map_name =
      user_space_instrumentation_addresses.GetInjectedLibraryMapName();
  const uint64_t* library_frame_ptr =
      std::find_if(callchain + 1, callchain + callchain_size,
                   [&maps, &injected_library_map_name](uint64_t frame) {
                     std::shared_ptr<unwindstack::MapInfo> map_info = maps.Find(frame);
                     return map_info != nullptr && map_info->name() == injected_library_map_name;
                   });
  if (library_frame_ptr == callchain + callchain_size) {
    return false;
  }
  return std::any_of(
      library_frame_ptr + 1, callchain + callchain_size,
      [&user_space_instrumentation_addresses](uint64_t frame) {
        return user_space_instrumentation_addresses.IsInEntryOrReturnTrampoline(frame);
      });
}

void UprobesUnwindingVisitor::SendFullAddressInfoToListener(
    const unwindstack::FrameData& libunwindstack_frame) {
  auto [unused_it, inserted] = known_linux_address_infos_.insert(libunwindstack_frame.pc);
  if (!inserted) {
    return;
  }

  FullAddressInfo address_info;
  address_info.set_absolute_address(libunwindstack_frame.pc);

  // Careful: unwindstack::FrameData::map_info might contain nullptr.
  if (libunwindstack_frame.map_info != nullptr) {
    address_info.set_module_name(libunwindstack_frame.map_info->name());
  }

  // For addresses falling directly inside u(ret)probes code, unwindstack::FrameData has limited
  // information. Nonetheless, we can send a perfectly meaningful FullAddressInfo, treating
  // u(ret)probes code as a single function. This makes sense as the only affected virtual addresses
  // I observed are 0x7fffffffe000 (~1% of uprobes addresses) and 0x7fffffffe001 (~99%). This way
  // the client can show more information for such a frame, in particular when associated with the
  // corresponding unwinding error.
  if (libunwindstack_frame.map_info != nullptr &&
      libunwindstack_frame.map_info->name() == "[uprobes]") {
    address_info.set_function_name("[uprobes]");
    address_info.set_offset_in_function(libunwindstack_frame.pc -
                                        libunwindstack_frame.map_info->start());
  } else {
    address_info.set_function_name(libunwindstack_frame.function_name);
    address_info.set_offset_in_function(libunwindstack_frame.function_offset);
  }

  ORBIT_CHECK(listener_ != nullptr);
  listener_->OnAddressInfo(std::move(address_info));
}

static inline bool IsPcInFunctionsToStopAt(
    const std::map<uint64_t, uint64_t>* absolute_address_to_size_of_functions_to_stop_at,
    uint64_t pc) {
  if (absolute_address_to_size_of_functions_to_stop_at == nullptr) {
    return false;
  }
  auto function_it = absolute_address_to_size_of_functions_to_stop_at->upper_bound(pc);
  if (function_it == absolute_address_to_size_of_functions_to_stop_at->begin()) {
    return false;
  }

  --function_it;

  uint64_t function_start = function_it->first;
  ORBIT_CHECK(function_start <= pc);
  uint64_t size = function_it->second;
  return (pc < function_start + size);
}

orbit_grpc_protos::Callstack::CallstackType
UprobesUnwindingVisitor::ComputeCallstackTypeFromStackSample(
    const LibunwindstackResult& libunwindstack_result) {
  if (libunwindstack_result.frames().front().map_info != nullptr &&
      libunwindstack_result.frames().front().map_info->name() == "[uprobes]") {
    // Some samples can actually fall inside u(ret)probes code. They cannot be unwound by
    // libunwindstack (even when the unwinding is reported as successful, the result is wrong).
    if (samples_in_uretprobes_counter_ != nullptr) {
      ++(*samples_in_uretprobes_counter_);
    }
    return Callstack::kInUprobes;
  }
  if (user_space_instrumentation_addresses_ != nullptr &&
      CallstackIsInUserSpaceInstrumentation(libunwindstack_result.frames(),
                                            *user_space_instrumentation_addresses_)) {
    // Like the previous case, but for user space instrumentation. This is harder to detect, as we
    // have to consider whether the sample:
    // - fell directly inside a user space instrumentation trampoline (entry or return); or
    // - fell inside liborbituserspaceinstrumentation.so or a module called by this, AND also
    //   includes a previous frame corresponding to a trampoline, usually where unwinding stopped
    //   (otherwise we could exclude other samples in the library that don't come from a
    //   trampoline).
    // We don't simply check if any frame is in the trampoline as we want to distinguish from the
    // kCallstackPatchingFailed case below.
    return Callstack::kInUserSpaceInstrumentation;
  }
  if (libunwindstack_result.frames().size() > 1 &&
      ((libunwindstack_result.frames().back().map_info != nullptr &&
        libunwindstack_result.frames().back().map_info->name() == "[uprobes]") ||
       (user_space_instrumentation_addresses_ != nullptr &&
        user_space_instrumentation_addresses_->IsInReturnTrampoline(
            libunwindstack_result.frames().back().pc)))) {
    // If unwinding stops at a [uprobes] frame or at a frame corresponding to a user space
    // instrumentation return trampoline (this is usually reported as an unwinding error, but not
    // always, at least for uprobes), it means that patching the stack with
    // UprobesReturnAddressManager::PatchSample wasn't (completely) successful (we cannot detect
    // this before actually unwinding). This easily happens at the beginning of the capture, when we
    // missed the first uprobes, but also if some perf_event_open events are lost or discarded.
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    return Callstack::kCallstackPatchingFailed;
  }
  if (!libunwindstack_result.IsSuccess() ||
      (libunwindstack_result.frames().size() == 1 &&
       !IsPcInFunctionsToStopAt(absolute_address_to_size_of_functions_to_stop_at_,
                                libunwindstack_result.frames().at(0).pc))) {
    // Callstacks with only one frame (the sampled address) are also unwinding errors, that were not
    // reported as such by LibunwindstackUnwinder::Unwind.
    // Note that this doesn't exclude samples inside the main function of any thread as the main
    // function is never the outermost frame. For example, for the main thread the outermost
    // function is _start, followed by __libc_start_main. For other threads, the outermost function
    // is clone.
    // The only exception are callstacks where the single frame is inside a function we forced the
    // unwinder to stop at (e.g. __wine_syscall_dispatcher).
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    return Callstack::kDwarfUnwindingError;
  }

  return Callstack::kComplete;
}

template <typename StackPerfEventDataT>
bool UprobesUnwindingVisitor::UnwindStack(const StackPerfEventDataT& event_data,
                                          Callstack* resulting_callstack,
                                          bool offline_memory_only) {
  ORBIT_CHECK(listener_ != nullptr);
  ORBIT_CHECK(current_maps_ != nullptr);

  return_address_manager_->PatchSample(event_data.GetCallstackTid(), event_data.GetRegisters().sp,
                                       event_data.GetMutableStackData(), event_data.GetStackSize());

  StackSliceView event_stack_slice{event_data.GetRegisters().sp, event_data.GetStackSize(),
                                   event_data.GetStackData()};
  std::vector<StackSliceView> stack_slices{event_stack_slice};
  const auto& stream_id_to_user_stack =
      thread_id_stream_id_to_stack_slices_.find(event_data.GetCallstackTid());
  if (stream_id_to_user_stack != thread_id_stream_id_to_stack_slices_.end()) {
    for (const auto& [unused_stream_id, user_stack_slice] : stream_id_to_user_stack->second) {
      stack_slices.emplace_back(user_stack_slice.start_address, user_stack_slice.size,
                                user_stack_slice.data.get());
    }
  }

  // There might be rare cases where the callstack's pid is "-1". This happens on callstacks on
  // "sched out" switches where the thread exits. This is not a big problem for unwinding, as
  // the process id is only used to read from the process' memory as a fallback to the collected
  // stack slice. When actually attempting to read from pid "-1" we will produce an unwinding error.
  // But this is not likely to happen.
  // TODO(b/246519821) It would be possible to retrieve the information from
  //  SwitchesStatesNamesVisitor::GetPidOfTid, but this requires major refactoring.
  LibunwindstackResult libunwindstack_result =
      unwinder_->Unwind(event_data.GetCallstackPidOrMinusOne(), current_maps_->Get(),
                        event_data.GetRegistersAsArray(), stack_slices, offline_memory_only);

  if (libunwindstack_result.frames().empty()) {
    // Even with unwinding errors this is not expected because we should at least get the program
    // counter. Do nothing in case this doesn't hold for a reason we don't know.
    ORBIT_ERROR("Unwound callstack has no frames");
    return false;
  }

  resulting_callstack->set_type(ComputeCallstackTypeFromStackSample(libunwindstack_result));
  for (const unwindstack::FrameData& libunwindstack_frame : libunwindstack_result.frames()) {
    SendFullAddressInfoToListener(libunwindstack_frame);
    resulting_callstack->add_pcs(libunwindstack_frame.pc);
  }

  ORBIT_CHECK(!resulting_callstack->pcs().empty());
  return true;
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const StackSamplePerfEventData& event_data) {
  FullCallstackSample sample;
  sample.set_pid(event_data.pid);
  sample.set_tid(event_data.tid);
  sample.set_timestamp_ns(event_timestamp);

  const bool success = UnwindStack(event_data, sample.mutable_callstack());

  if (!success) {
    return;
  }

  listener_->OnCallstackSample(std::move(sample));
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const SchedWakeupWithStackPerfEventData& event_data) {
  ThreadStateSliceCallstack thread_state_slice_callstack;
  thread_state_slice_callstack.set_thread_state_slice_tid(event_data.woken_tid);
  thread_state_slice_callstack.set_timestamp_ns(event_timestamp);

  const bool success = UnwindStack(event_data, thread_state_slice_callstack.mutable_callstack(),
                                   /*offline_memory_only=*/true);

  if (!success) {
    return;
  }

  listener_->OnThreadStateSliceCallstack(std::move(thread_state_slice_callstack));
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const SchedSwitchWithStackPerfEventData& event_data) {
  ThreadStateSliceCallstack thread_state_slice_callstack;
  thread_state_slice_callstack.set_thread_state_slice_tid(event_data.prev_tid);
  thread_state_slice_callstack.set_timestamp_ns(event_timestamp);

  const bool success = UnwindStack(event_data, thread_state_slice_callstack.mutable_callstack(),
                                   /*offline_memory_only=*/true);

  if (!success) {
    return;
  }

  listener_->OnThreadStateSliceCallstack(std::move(thread_state_slice_callstack));
}

template <typename CallchainPerfEventDataT>
orbit_grpc_protos::Callstack::CallstackType
UprobesUnwindingVisitor::ComputeCallstackTypeFromCallchainAndPatch(
    const CallchainPerfEventDataT& event_data) {
  // Callstacks with only two frames (the first is in the kernel, the second is the sampled address)
  // are unwinding errors.
  // Note that this doesn't exclude samples inside the main function of any thread as the main
  // function is never the outermost frame. For example, for the main thread the outermost function
  // is _start, followed by __libc_start_main. For other threads, the outermost function is clone.
  if (event_data.GetCallchainSize() == 2) {
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    return Callstack::kFramePointerUnwindingError;
  }

  uint64_t top_ip = event_data.GetCallchain()[1];

  // Some samples can actually fall inside u(ret)probes code. Set their type accordingly, as we
  // don't want to show the unnamed uprobes module in the samples.
  std::shared_ptr<unwindstack::MapInfo> top_ip_map_info = current_maps_->Find(top_ip);
  if (top_ip_map_info != nullptr && top_ip_map_info->name() == "[uprobes]") {
    if (samples_in_uretprobes_counter_ != nullptr) {
      ++(*samples_in_uretprobes_counter_);
    }
    return Callstack::kInUprobes;
  }

  // Similar to the previous case, but for user space instrumentation. We consider whether a sample:
  // - fell directly inside a user space instrumentation trampoline (entry or return); or
  // - fell inside liborbituserspaceinstrumentation.so or a module called by this, AND also
  //   includes a previous frame corresponding to a trampoline, usually where unwinding stopped.
  // We don't simply check if any frame is in the trampoline as that's normal before calling
  // PatchCallchain.
  if (user_space_instrumentation_addresses_ != nullptr &&
      CallchainIsInUserSpaceInstrumentation(event_data.GetCallchain(),
                                            event_data.GetCallchainSize(), *current_maps_,
                                            *user_space_instrumentation_addresses_)) {
    return Callstack::kInUserSpaceInstrumentation;
  }

  // The leaf function is not guaranteed to have the frame pointer for all our targets. Though, we
  // assume that $rbp remains untouched by the leaf functions, such that we can rely on
  // perf_event_open to give us "almost" correct callstacks (the caller of the leaf function will be
  // missing). We do a plausibility check for this assumption by checking if the callstack only
  // contains executable code.
  for (uint64_t frame_index = 1; frame_index < event_data.GetCallchainSize(); ++frame_index) {
    std::shared_ptr<unwindstack::MapInfo> map_info =
        current_maps_->Find(event_data.GetCallchain()[frame_index]);
    if (map_info == nullptr || (map_info->flags() & PROT_EXEC) == 0) {
      if (unwind_error_counter_ != nullptr) {
        ++(*unwind_error_counter_);
      }
      return Callstack::kFramePointerUnwindingError;
    }
  }

  Callstack::CallstackType leaf_function_patching_status =
      leaf_function_call_manager_->PatchCallerOfLeafFunction(&event_data, current_maps_, unwinder_);
  if (leaf_function_patching_status != Callstack::kComplete) {
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    return leaf_function_patching_status;
  }

  // Apparently quite a corner case, but easy to observe: the library injected by user space
  // instrumentation didn't appear in the callchain because it called a leaf function in another
  // module, but after calling PatchCallerOfLeafFunction it's now the second innermost frame.
  if (user_space_instrumentation_addresses_ != nullptr && event_data.GetCallchainSize() >= 4) {
    std::shared_ptr<unwindstack::MapInfo> second_ip_map_info =
        current_maps_->Find(event_data.GetCallchain()[2]);
    if (second_ip_map_info != nullptr &&
        second_ip_map_info->name() ==
            user_space_instrumentation_addresses_->GetInjectedLibraryMapName() &&
        // Verify that the sample actually came from a user space instrumentation trampoline.
        std::any_of(
            event_data.GetCallchain() + 3,
            event_data.GetCallchain() + event_data.GetCallchainSize(), [this](uint64_t frame) {
              return user_space_instrumentation_addresses_->IsInEntryOrReturnTrampoline(frame);
            })) {
      return Callstack::kInUserSpaceInstrumentation;
    }
  }

  if (!return_address_manager_->PatchCallchain(event_data.GetCallstackTid(), event_data.ips.get(),
                                               event_data.GetCallchainSize(), current_maps_)) {
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }

    return Callstack::kCallstackPatchingFailed;
  }

  return Callstack::kComplete;
}

template <typename CallchainPerfEventDataT>
bool UprobesUnwindingVisitor::VisitCallchainEvent(const CallchainPerfEventDataT& event_data,
                                                  Callstack* resulting_callstack) {
  // The top of a callchain is always inside the kernel code and we don't expect samples to be only
  // inside the kernel. Do nothing in case this happens anyway for some reason.
  if (event_data.GetCallchainSize() <= 1) {
    ORBIT_ERROR("Callchain has only %lu frames", event_data.GetCallchainSize());
    return false;
  }

  resulting_callstack->set_type(ComputeCallstackTypeFromCallchainAndPatch(event_data));

  // Skip the first frame as the top of a perf_event_open callchain is always inside kernel code.
  resulting_callstack->add_pcs(event_data.GetCallchain()[1]);
  // Only the address of the top of the stack is correct. Frame-based unwinding
  // uses the return address of a function call as the caller's address.
  // However, the actual address of the call instruction is before that.
  // As we don't know the size of the call instruction, we subtract 1 from the
  // return address. This way we fall into the range of the call instruction.
  // Note: This is also done the same way in Libunwindstack.
  for (uint64_t frame_index = 2; frame_index < event_data.GetCallchainSize(); ++frame_index) {
    resulting_callstack->add_pcs(event_data.GetCallchain()[frame_index] - 1);
  }

  return true;
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const CallchainSamplePerfEventData& event_data) {
  ORBIT_CHECK(listener_ != nullptr);
  ORBIT_CHECK(current_maps_ != nullptr);

  FullCallstackSample sample;
  sample.set_pid(event_data.pid);
  sample.set_tid(event_data.tid);
  sample.set_timestamp_ns(event_timestamp);
  Callstack* callstack = sample.mutable_callstack();

  bool success = VisitCallchainEvent(event_data, callstack);
  if (!success) {
    return;
  }

  ORBIT_CHECK(!callstack->pcs().empty());
  listener_->OnCallstackSample(std::move(sample));
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const SchedWakeupWithCallchainPerfEventData& event_data) {
  ThreadStateSliceCallstack thread_state_slice_callstack;
  thread_state_slice_callstack.set_thread_state_slice_tid(event_data.woken_tid);
  thread_state_slice_callstack.set_timestamp_ns(event_timestamp);

  const bool success =
      VisitCallchainEvent(event_data, thread_state_slice_callstack.mutable_callstack());

  if (!success) {
    return;
  }

  listener_->OnThreadStateSliceCallstack(std::move(thread_state_slice_callstack));
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const SchedSwitchWithCallchainPerfEventData& event_data) {
  ThreadStateSliceCallstack thread_state_slice_callstack;
  thread_state_slice_callstack.set_thread_state_slice_tid(event_data.prev_tid);
  thread_state_slice_callstack.set_timestamp_ns(event_timestamp);

  const bool success =
      VisitCallchainEvent(event_data, thread_state_slice_callstack.mutable_callstack());

  if (!success) {
    return;
  }

  listener_->OnThreadStateSliceCallstack(std::move(thread_state_slice_callstack));
}

void UprobesUnwindingVisitor::OnUprobes(
    uint64_t timestamp_ns, pid_t tid, uint32_t cpu, uint64_t sp, uint64_t ip,
    uint64_t return_address, std::optional<perf_event_sample_regs_user_sp_ip_arguments> registers,
    uint64_t function_id) {
  ORBIT_CHECK(listener_ != nullptr);

  // We are seeing that, on thread migration, uprobe events can sometimes be
  // duplicated: the duplicate uprobe event will have the same stack pointer and
  // instruction pointer as the previous uprobe, but different cpu. In that
  // situation, we discard the second uprobe event.
  // We also discard a uprobe event in the general case of strictly-increasing
  // stack pointers, as for a given thread's sequence of u(ret)probe events, two
  // consecutive uprobe events must be associated with non-increasing stack
  // pointers (the stack grows towards lower addresses).

  // Duplicate uprobe detection.
  std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>& uprobe_sps_ips_cpus =
      uprobe_sps_ips_cpus_per_thread_[tid];
  if (!uprobe_sps_ips_cpus.empty()) {
    uint64_t last_uprobe_sp = std::get<0>(uprobe_sps_ips_cpus.back());
    uint64_t last_uprobe_ip = std::get<1>(uprobe_sps_ips_cpus.back());
    uint32_t last_uprobe_cpu = std::get<2>(uprobe_sps_ips_cpus.back());
    uprobe_sps_ips_cpus.pop_back();
    if (sp > last_uprobe_sp) {
      ORBIT_ERROR("MISSING URETPROBE OR DUPLICATE UPROBE");
      return;
    }
    if (sp == last_uprobe_sp && ip == last_uprobe_ip && cpu != last_uprobe_cpu) {
      ORBIT_ERROR("Duplicate uprobe on thread migration");
      return;
    }
  }
  uprobe_sps_ips_cpus.emplace_back(sp, ip, cpu);

  function_call_manager_->ProcessFunctionEntry(tid, function_id, timestamp_ns, registers);

  return_address_manager_->ProcessFunctionEntry(tid, sp, return_address);
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const UprobesPerfEventData& event_data) {
  OnUprobes(event_timestamp, event_data.tid, event_data.cpu, event_data.sp, event_data.ip,
            event_data.return_address,
            /*registers=*/std::nullopt, event_data.function_id);
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const UprobesWithArgumentsPerfEventData& event_data) {
  OnUprobes(event_timestamp, event_data.tid, event_data.cpu, event_data.regs.sp, event_data.regs.ip,
            event_data.return_address, event_data.regs, event_data.function_id);
}

void UprobesUnwindingVisitor::OnUretprobes(uint64_t timestamp_ns, pid_t pid, pid_t tid,
                                           std::optional<uint64_t> ax) {
  ORBIT_CHECK(listener_ != nullptr);

  // Duplicate uprobe detection.
  std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>& uprobe_sps_ips_cpus =
      uprobe_sps_ips_cpus_per_thread_[tid];
  if (!uprobe_sps_ips_cpus.empty()) {
    uprobe_sps_ips_cpus.pop_back();
  }

  std::optional<FunctionCall> function_call =
      function_call_manager_->ProcessFunctionExit(pid, tid, timestamp_ns, ax);
  if (function_call.has_value()) {
    listener_->OnFunctionCall(std::move(function_call.value()));
  }

  return_address_manager_->ProcessFunctionExit(tid);
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const UretprobesPerfEventData& event_data) {
  OnUretprobes(event_timestamp, event_data.pid, event_data.tid, /*ax=*/std::nullopt);
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const UretprobesWithReturnValuePerfEventData& event_data) {
  OnUretprobes(event_timestamp, event_data.pid, event_data.tid, event_data.rax);
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const UserSpaceFunctionEntryPerfEventData& event_data) {
  function_call_manager_->ProcessFunctionEntry(event_data.tid, event_data.function_id,
                                               event_timestamp, std::nullopt);

  return_address_manager_->ProcessFunctionEntry(event_data.tid, event_data.sp,
                                                event_data.return_address);
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const UserSpaceFunctionExitPerfEventData& event_data) {
  std::optional<FunctionCall> function_call = function_call_manager_->ProcessFunctionExit(
      event_data.pid, event_data.tid, event_timestamp, std::nullopt);
  if (function_call.has_value()) {
    listener_->OnFunctionCall(std::move(function_call.value()));
  }

  return_address_manager_->ProcessFunctionExit(event_data.tid);
}

void UprobesUnwindingVisitor::Visit(uint64_t /*event_timestamp*/,
                                    const UprobesWithStackPerfEventData& event_data) {
  StackSlice stack_slice{.start_address = event_data.GetRegisters().sp,
                         .size = event_data.dyn_size,
                         .data = std::move(event_data.data)};
  absl::flat_hash_map<uint64_t, StackSlice>& stream_id_to_stack =
      thread_id_stream_id_to_stack_slices_[event_data.tid];
  stream_id_to_stack.insert_or_assign(event_data.stream_id, std::move(stack_slice));
}

[[nodiscard]] static std::shared_ptr<unwindstack::MapInfo> FindFileMapInfoPrecedingAnonMapInfo(
    const std::shared_ptr<unwindstack::MapInfo>& anon_map_info) {
  ORBIT_CHECK(anon_map_info->name().empty());
  // Scan the maps backwards until a file mapping is encountered, by skipping over anonymous
  // mappings.
  // Note that when the first character of a map name is '[', the mapping is a special one like
  // [stack], [heap], etc.: even if such a mapping has a name, it's still not a file mapping.
  std::shared_ptr<unwindstack::MapInfo> map_info_it = anon_map_info->prev_map();
  while (map_info_it != nullptr &&
         (map_info_it->name().empty() || map_info_it->name().c_str()[0] == '[')) {
    map_info_it = map_info_it->prev_map();
  }

  if (map_info_it == nullptr || (map_info_it->flags() & unwindstack::MAPS_FLAGS_DEVICE_MAP) != 0) {
    // This is unexpected if anon_map_info was detected to belong to a PE.
    return nullptr;
  }
  return map_info_it;
}

[[nodiscard]] static std::shared_ptr<unwindstack::MapInfo> FindFirstMapInfoForSameFile(
    const std::shared_ptr<unwindstack::MapInfo>& file_map_info) {
  const std::string file_path = file_map_info->name();
  ORBIT_CHECK(!file_path.empty());
  std::shared_ptr<unwindstack::MapInfo> first_map_info_for_file_path;
  // Scan the maps backwards. Stop when a file mapping for a different file is found. Skip over
  // anonymous mappings.
  // Note that when the first character of a map name is '[', the mapping is a special one like
  // [stack], [heap], etc.: even if such a mapping has a name, it's still not a file mapping.
  std::shared_ptr<unwindstack::MapInfo> map_info_it = file_map_info;
  while (map_info_it != nullptr &&
         (map_info_it->name().empty() || map_info_it->name().c_str()[0] == '[' ||
          map_info_it->name() == file_path)) {
    if (map_info_it->name() == file_path) {
      first_map_info_for_file_path = map_info_it;
    }
    map_info_it = map_info_it->prev_map();
  }
  // Assigned at least by the first iteration of the loop.
  ORBIT_CHECK(first_map_info_for_file_path != nullptr);
  return first_map_info_for_file_path;
}

[[nodiscard]] static std::pair<uint64_t, uint64_t>
FindExecutableAddressRangeForSameFileFromFirstMapInfo(
    const std::shared_ptr<unwindstack::MapInfo>& first_map_info, const unwindstack::PeCoff* pe,
    const std::shared_ptr<unwindstack::Memory>& process_memory) {
  uint64_t min_exec_map_start = std::numeric_limits<uint64_t>::max();
  uint64_t max_exec_map_end = 0;

  // Scan the maps forward. Stop when a file mapping for a different file is found.
  // Note that when the first character of a map name is '[', the mapping is a special one like
  // [stack], [heap], etc.: even if such a mapping has a name, it's still not a file mapping.
  std::shared_ptr<unwindstack::MapInfo> map_info_it = first_map_info;
  while (map_info_it != nullptr &&
         (map_info_it->name().empty() || map_info_it->name().c_str()[0] == '[' ||
          map_info_it->name() == first_map_info->name())) {
    const bool is_executable_file_mapping = ((map_info_it->flags() & PROT_EXEC) != 0) &&
                                            (map_info_it->name() == first_map_info->name());

    const bool is_anonymous_executable_mapping_of_pe =
        (pe != nullptr) && ((map_info_it->flags() & PROT_EXEC) != 0) &&
        (map_info_it->name().empty()) &&
        (dynamic_cast<unwindstack::PeCoff*>(
             map_info_it->GetObject(process_memory, unwindstack::ARCH_X86_64)) != nullptr);

    if (is_executable_file_mapping || is_anonymous_executable_mapping_of_pe) {
      min_exec_map_start = std::min(min_exec_map_start, map_info_it->start());
      max_exec_map_end = std::max(max_exec_map_end, map_info_it->end());
    }
    map_info_it = map_info_it->next_map();
  }

  return {min_exec_map_start, max_exec_map_end};
}

// We use PERF_RECORD_MMAP events to keep current_maps_ up to date, which is necessary for
// unwinding.
//
// In addition, whenever a new executable mapping appears, it's possible that a module has been
// newly mapped or has been re-mapped differently. We want to send a ModuleUpdateEvent in these
// cases, so that the client has an up-to-date snapshot of the modules of the target. Ideally, for
// each new executable file mapping we would send a ModuleUpdateEvent with the address range and
// file for that mapping.
//
// But just like for orbit_object_utils::ReadModulesFromMaps, things are more complicated. We
// observed that in some cases a single loadable segment of an ELF file or a single executable
// section of a PE can be loaded into memory with multiple adjacent file mappings. In addition, some
// PEs can have multiple executable sections. And finally, the executable sections (and all other
// sections) of a PE can have an offset in the file that doesn't fulfill the requirements of mmap
// for file mappings, in which case Wine has to create an anonymous mapping and copy the section
// into it.
//
// In all these cases, we want to create a ModuleUpdateEvent with an address range that includes
// all the executable mappings of the module. To find them, we proceed as follows:
// - We start from the new executable mapping.
// - If this mapping is anonymous, and we know that it belongs to a PE, we scan the maps backwards
//   to find the file; if it does not belong to a PE, we stop and won't send any ModuleUpdateEvent.
// - We scan the maps backwards to find the file mapping that is the start of the module.
// - From here, we scan the maps forwards to find all the executable mappings that belong to the
//   module; if the module is a PE, we also have to consider anonymous mappings and detect whether
//   they actually belong to the PE.
//
// Note that, just like in orbit_object_utils::ReadModulesFromMaps:
// - The ModuleInfo in the ModuleUpdateEvent will carry executable_segment_offset with the
//   assumption that the value of ObjectFile::GetExecutableSegmentOffset correspond to the *first*
//   executable mapping.
// - In the case of multiple executable sections, these are not necessarily adjacent, while the
//   ModuleInfo in the ModuleUpdateEvent as constructed will represent a single contiguous address
//   range. We believe this is fine.
void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp, const MmapPerfEventData& event_data) {
  ORBIT_CHECK(listener_ != nullptr);
  ORBIT_CHECK(current_maps_ != nullptr);

  // PERF_RECORD_MMAP events do not contain the flags, but only distinguish between executable and
  // non-executable. This is all we need, so simply assume PROT_READ | PROT_EXEC for executable
  // mappings and PROT_READ for non-executable mappings. If we wanted the exact flags, we could
  // switch to PERF_RECORD_MMAP2 events.
  if (!event_data.executable) {
    current_maps_->AddAndSort(event_data.address, event_data.address + event_data.length,
                              event_data.page_offset, PROT_READ, event_data.filename);
  } else {
    // Note that this case also covers the addition of the [uprobes] map that gets created the first
    // time a uprobe is hit in a process. It is important that current_maps_ contains it. For
    // example, UprobesReturnAddressManager::PatchCallchain needs it to check whether a program
    // counter is inside the uprobes map, and UprobesUnwindingVisitor::Visit(uint64_t, const
    // StackSamplePerfEventData&) needs it to throw away incorrectly-unwound samples. This is a case
    // where the flags are incorrect, because the [uprobes] map is not readable and only executable,
    // but again, this doesn't matter.
    current_maps_->AddAndSort(event_data.address, event_data.address + event_data.length,
                              event_data.page_offset, PROT_READ | PROT_EXEC, event_data.filename);
  }

  if (!event_data.executable) {
    // Don't try to send a ModuleUpdateEvent when non-executable mappings are added.
    return;
  }
  if (!event_data.filename.empty() && event_data.filename[0] == '[') {
    // The new mapping is a "special" executable mapping like [vdso], [vsyscall], [uprobes].
    return;
  }

  const std::shared_ptr<unwindstack::MapInfo> added_map_info =
      current_maps_->Find(event_data.address);
  ORBIT_CHECK(added_map_info != nullptr);  // This is the mapping added with AddAndSort above.

  const std::shared_ptr<unwindstack::Memory> process_memory =
      unwindstack::Memory::CreateProcessMemory(event_data.pid);
  auto* const pe = dynamic_cast<unwindstack::PeCoff*>(
      added_map_info->GetObject(process_memory, unwindstack::ARCH_X86_64));

  // If this is an anonymous executable mapping, we verify whether it belongs to a section of a PE
  // that was mapped anonymously by Wine because its alignment doesn't obey the requirements of
  // mmap. If this is not the case, we don't try to send a ModuleUpdateEvent because of this map.
  if (event_data.filename.empty() && pe == nullptr) {
    return;
  }

  std::shared_ptr<unwindstack::MapInfo> closest_file_map_info;
  if (!event_data.filename.empty()) {
    closest_file_map_info = added_map_info;
  } else {
    // This anonymous executable map corresponds to a PE. We know that this was detected from the
    // previous file mapping, see MapInfo::GetFileMemoryFromAnonExecMapIfPeCoffTextSection. Find
    // such mapping and get its file. Note that this assumes that at least the headers of the PE
    // are already mapped (with a non-executable file mapping).
    closest_file_map_info = FindFileMapInfoPrecedingAnonMapInfo(added_map_info);
    if (closest_file_map_info == nullptr) {
      ORBIT_ERROR("No file mapping found preceding anon exec map at %#x-%#x that belongs to a PE",
                  added_map_info->start(), added_map_info->end());
      return;
    }
  }

  // Find the first file mapping with the same name (file path) as the file mapping we just found.
  // For ELF files, this should correspond to (the first part of) the first loadable segment.
  // For PEs, this should correspond to the headers.
  const std::shared_ptr<unwindstack::MapInfo> first_map_info_for_module =
      FindFirstMapInfoForSameFile(closest_file_map_info);
  const std::string module_path = closest_file_map_info->name();

  // We want to find the first and the last executable map for this file so that we can create a
  // ModuleUpdateEvent that encompasses all of them.
  auto [min_exec_map_start, max_exec_map_end] =
      FindExecutableAddressRangeForSameFileFromFirstMapInfo(first_map_info_for_module, pe,
                                                            process_memory);
  if (min_exec_map_start >= max_exec_map_end) {
    return;
  }

  ErrorMessageOr<orbit_grpc_protos::ModuleInfo> module_info_or_error =
      orbit_module_utils::CreateModule(module_path, min_exec_map_start, max_exec_map_end);
  if (module_info_or_error.has_error()) {
    ORBIT_ERROR("Unable to create module: %s", module_info_or_error.error().message());
    return;
  }
  orbit_grpc_protos::ModuleUpdateEvent module_update_event;
  module_update_event.set_pid(event_data.pid);
  module_update_event.set_timestamp_ns(event_timestamp);
  *module_update_event.mutable_module() = std::move(module_info_or_error.value());
  listener_->OnModuleUpdate(std::move(module_update_event));
}

}  // namespace orbit_linux_tracing
