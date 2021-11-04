// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UprobesUnwindingVisitor.h"

#include <asm/perf_regs.h>
#include <sys/mman.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/SharedString.h>
#include <unwindstack/Unwinder.h>

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "LeafFunctionCallManager.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "capture.pb.h"
#include "module.pb.h"

namespace orbit_linux_tracing {

using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::FullAddressInfo;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FunctionCall;

static void SendFullAddressInfoToListener(TracerListener* listener,
                                          const unwindstack::FrameData& libunwindstack_frame) {
  CHECK(listener != nullptr);

  FullAddressInfo address_info;
  address_info.set_absolute_address(libunwindstack_frame.pc);
  address_info.set_function_name(libunwindstack_frame.function_name);
  address_info.set_offset_in_function(libunwindstack_frame.function_offset);
  address_info.set_module_name(libunwindstack_frame.map_name);

  listener->OnAddressInfo(std::move(address_info));
}

// For addresses falling directly inside u(ret)probes code, unwindstack::FrameData has limited
// information. Nonetheless, we can send a perfectly meaningful FullAddressInfo, treating
// u(ret)probes code as a single function. This makes sense as the only affected virtual addresses I
// observed are 0x7fffffffe000 (~1% of uprobes addresses) and 0x7fffffffe001 (~99%). This way the
// client can show more information for such a frame, in particular when associated with the
// corresponding unwinding error.
static void SendUprobesFullAddressInfoToListener(
    TracerListener* listener, const unwindstack::FrameData& libunwindstack_frame) {
  CHECK(listener != nullptr);

  FullAddressInfo address_info;
  address_info.set_absolute_address(libunwindstack_frame.pc);
  address_info.set_function_name("[uprobes]");
  address_info.set_offset_in_function(libunwindstack_frame.pc - libunwindstack_frame.map_start);
  address_info.set_module_name("[uprobes]");

  listener->OnAddressInfo(std::move(address_info));
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const StackSamplePerfEventData& event_data) {
  CHECK(listener_ != nullptr);
  CHECK(current_maps_ != nullptr);

  return_address_manager_->PatchSample(event_data.tid, event_data.GetRegisters()[PERF_REG_X86_SP],
                                       event_data.GetMutableStackData(), event_data.GetStackSize());

  LibunwindstackResult libunwindstack_result =
      unwinder_->Unwind(event_data.pid, current_maps_->Get(), event_data.GetRegisters(),
                        event_data.GetStackData(), event_data.GetStackSize());

  if (libunwindstack_result.frames().empty()) {
    // Even with unwinding errors this is not expected because we should at least get the program
    // counter. Do nothing in case this doesn't hold for a reason we don't know.
    ERROR("Unwound callstack has no frames");
    return;
  }

  FullCallstackSample sample;
  sample.set_pid(event_data.pid);
  sample.set_tid(event_data.tid);
  sample.set_timestamp_ns(event_timestamp);

  Callstack* callstack = sample.mutable_callstack();

  if (libunwindstack_result.frames().front().map_name == "[uprobes]") {
    // TODO(b/194704608): Also detect if the sample is inside a user space instrumentation entry
    //  trampoline or return trampoline. Note that the sample could also be inside the entry or exit
    //  payload in liborbituserspaceinstrumentation.so or a library called by it.
    //  For now, these samples will just be unwinding errors as we can't unwind past the trampoline.

    // Some samples can actually fall inside u(ret)probes code. They cannot be unwound by
    // libunwindstack (even when the unwinding is reported as successful, the result is wrong).
    if (samples_in_uretprobes_counter_ != nullptr) {
      ++(*samples_in_uretprobes_counter_);
    }
    callstack->set_type(Callstack::kInUprobes);
    SendUprobesFullAddressInfoToListener(listener_, libunwindstack_result.frames().front());
    callstack->add_pcs(libunwindstack_result.frames().front().pc);

  } else if (libunwindstack_result.frames().size() > 1 &&
             libunwindstack_result.frames().back().map_name == "[uprobes]") {
    // TODO(b/194704608): Also detect if unwinding stopped at a user space instrumentation return
    //  trampoline.

    // If unwinding stops at a [uprobes] frame (this is usually reported as an unwinding error, but
    // not always), it means that patching the stack with UprobesReturnAddressManager::PatchSample
    // wasn't (completely) successful (we cannot detect this before actually unwinding).
    // This easily happens at the beginning of the capture, when we missed the first uprobes, but
    // also if some perf_event_open events are lost or discarded.
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    callstack->set_type(Callstack::kCallstackPatchingFailed);
    SendFullAddressInfoToListener(listener_, libunwindstack_result.frames().front());
    callstack->add_pcs(libunwindstack_result.frames().front().pc);

  } else if (!libunwindstack_result.IsSuccess() || libunwindstack_result.frames().size() == 1) {
    // Callstacks with only one frame (the sampled address) are also unwinding errors, that were not
    // reported as such by LibunwindstackUnwinder::Unwind.
    // Note that this doesn't exclude samples inside the main function of any thread as the main
    // function is never the outermost frame. For example, for the main thread the outermost
    // function is _start, followed by __libc_start_main. For other threads, the outermost function
    // is clone.
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    callstack->set_type(Callstack::kDwarfUnwindingError);
    SendFullAddressInfoToListener(listener_, libunwindstack_result.frames().front());
    callstack->add_pcs(libunwindstack_result.frames().front().pc);

  } else {
    callstack->set_type(Callstack::kComplete);

    for (const unwindstack::FrameData& libunwindstack_frame : libunwindstack_result.frames()) {
      SendFullAddressInfoToListener(listener_, libunwindstack_frame);
      callstack->add_pcs(libunwindstack_frame.pc);
    }
  }

  CHECK(!callstack->pcs().empty());
  listener_->OnCallstackSample(std::move(sample));
}

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp,
                                    const CallchainSamplePerfEventData& event_data) {
  CHECK(listener_ != nullptr);
  CHECK(current_maps_ != nullptr);

  // The top of a callchain is always inside the kernel code and we don't expect samples to be only
  // inside the kernel. Do nothing in case this happens anyway for some reason.
  if (event_data.GetCallchainSize() <= 1) {
    ERROR("Callchain has only %lu frames", event_data.GetCallchainSize());
    return;
  }

  FullCallstackSample sample;
  sample.set_pid(event_data.pid);
  sample.set_tid(event_data.tid);
  sample.set_timestamp_ns(event_timestamp);

  Callstack* callstack = sample.mutable_callstack();

  // Callstacks with only two frames (the first is in the kernel, the second is the sampled address)
  // are unwinding errors.
  // Note that this doesn't exclude samples inside the main function of any thread as the main
  // function is never the outermost frame. For example, for the main thread the outermost function
  // is _start, followed by __libc_start_main. For other threads, the outermost function is clone.
  if (event_data.GetCallchainSize() == 2) {
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    callstack->set_type(Callstack::kFramePointerUnwindingError);
    callstack->add_pcs(event_data.GetCallchain()[1]);
    listener_->OnCallstackSample(std::move(sample));
    return;
  }

  uint64_t top_ip = event_data.GetCallchain()[1];
  unwindstack::MapInfo* top_ip_map_info = current_maps_->Find(top_ip);

  // Some samples can actually fall inside u(ret)probes code. Set their type accordingly, as we
  // don't want to show the unnamed uprobes module in the samples.
  if (top_ip_map_info == nullptr || top_ip_map_info->name() == "[uprobes]") {
    if (samples_in_uretprobes_counter_ != nullptr) {
      ++(*samples_in_uretprobes_counter_);
    }
    callstack->set_type(Callstack::kInUprobes);
    callstack->add_pcs(top_ip);
    listener_->OnCallstackSample(std::move(sample));
    return;
  }
  // TODO(b/194704608): Also detect if the sample is inside a user space instrumentation entry
  //  trampoline or return trampoline, the entry or return payloads in
  //  liborbituserspaceinstrumentation.so, or a library called by the payloads.

  // The leaf function is not guaranteed to have the frame pointer for all our targets. Though, we
  // assume that $rbp remains untouched by the leaf functions, such that we can rely on
  // perf_event_open to give us "almost" correct callstacks (the caller of the leaf function will be
  // missing). We do a plausibility check for this assumption by checking if the callstack only
  // contains executable code.
  // TODO(b/187690455): As soon as we actually have frame pointers in all non-leaf functions, we can
  //  report an unwinding error here. Till that point this check will always fail, because the
  //  caller of __libc_start_main will be invalid since libc doesn't have frame-pointers. This
  //  prevents us from testing the current implementation, which will have "almost" correct
  //  callstack.
  for (uint64_t frame_index = 1; frame_index < event_data.GetCallchainSize(); ++frame_index) {
    unwindstack::MapInfo* map_info = current_maps_->Find(event_data.GetCallchain()[frame_index]);
    if (map_info == nullptr || (map_info->flags() & PROT_EXEC) == 0) {
      break;
    }
  }

  Callstack::CallstackType leaf_function_patching_status =
      leaf_function_call_manager_->PatchCallerOfLeafFunction(&event_data, current_maps_, unwinder_);
  if (leaf_function_patching_status != Callstack::kComplete) {
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    callstack->set_type(leaf_function_patching_status);
    callstack->add_pcs(top_ip);
    listener_->OnCallstackSample(std::move(sample));
    return;
  }

  if (!return_address_manager_->PatchCallchain(event_data.tid, event_data.ips.get(),
                                               event_data.GetCallchainSize(), current_maps_)) {
    if (unwind_error_counter_ != nullptr) {
      ++(*unwind_error_counter_);
    }
    callstack->set_type(Callstack::kCallstackPatchingFailed);
    callstack->add_pcs(top_ip);
    listener_->OnCallstackSample(std::move(sample));
    return;
  }

  callstack->set_type(Callstack::kComplete);
  // Skip the first frame as the top of a perf_event_open callchain is always
  // inside kernel code.
  callstack->add_pcs(event_data.GetCallchain()[1]);
  // Only the address of the top of the stack is correct. Frame-based unwinding
  // uses the return address of a function call as the caller's address.
  // However, the actual address of the call instruction is before that.
  // As we don't know the size of the call instruction, we subtract 1 from the
  // return address. This way we fall into the range of the call instruction.
  // Note: This is also done the same way in Libunwindstack.
  for (uint64_t frame_index = 2; frame_index < event_data.GetCallchainSize(); ++frame_index) {
    callstack->add_pcs(event_data.GetCallchain()[frame_index] - 1);
  }

  CHECK(!callstack->pcs().empty());
  listener_->OnCallstackSample(std::move(sample));
}

void UprobesUnwindingVisitor::OnUprobes(
    uint64_t timestamp_ns, pid_t tid, uint32_t cpu, uint64_t sp, uint64_t ip,
    uint64_t return_address, std::optional<perf_event_sample_regs_user_sp_ip_arguments> registers,
    uint64_t function_id) {
  CHECK(listener_ != nullptr);

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
      ERROR("MISSING URETPROBE OR DUPLICATE UPROBE");
      return;
    }
    if (sp == last_uprobe_sp && ip == last_uprobe_ip && cpu != last_uprobe_cpu) {
      ERROR("Duplicate uprobe on thread migration");
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
  CHECK(listener_ != nullptr);

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

void UprobesUnwindingVisitor::Visit(uint64_t event_timestamp, const MmapPerfEventData& event_data) {
  CHECK(listener_ != nullptr);
  CHECK(current_maps_ != nullptr);

  // Obviously the uprobes map cannot be successfully processed by orbit_object_utils::CreateModule,
  // but it's important that current_maps_ contain it.
  // For example, UprobesReturnAddressManager::PatchCallchain needs it to check whether a program
  // counter is inside the uprobes map, and UprobesUnwindingVisitor::Visit( const
  // StackSamplePerfEvent*) needs it to throw away incorrectly-unwound samples.
  // As below we are only adding maps successfully parsed with orbit_object_utils::CreateModule, we
  // add the uprobes map manually. We are using the same values that that uprobes map would get if
  // unwindstack::BufferMaps was built by passing the full content of /proc/<pid>/maps to its
  // constructor.
  if (event_data.filename == "[uprobes]") {
    current_maps_->AddAndSort(event_data.address, event_data.address + event_data.length, 0,
                              PROT_EXEC, event_data.filename, INT64_MAX);
    return;
  }

  ErrorMessageOr<orbit_grpc_protos::ModuleInfo> module_info_or_error =
      orbit_object_utils::CreateModule(event_data.filename, event_data.address,
                                       event_data.address + event_data.length);
  if (module_info_or_error.has_error()) {
    ERROR("Unable to create module: %s", module_info_or_error.error().message());
    return;
  }

  auto& module_info = module_info_or_error.value();

  // For flags we assume PROT_READ and PROT_EXEC, MMAP event does not return flags.
  current_maps_->AddAndSort(module_info.address_start(), module_info.address_end(),
                            event_data.page_offset, PROT_READ | PROT_EXEC, event_data.filename,
                            module_info.load_bias());

  orbit_grpc_protos::ModuleUpdateEvent module_update_event;
  module_update_event.set_pid(event_data.pid);
  module_update_event.set_timestamp_ns(event_timestamp);
  *module_update_event.mutable_module() = std::move(module_info);

  listener_->OnModuleUpdate(std::move(module_update_event));
}

}  // namespace orbit_linux_tracing
