// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_UPROBES_RETURN_ADDRESS_MANAGER_H_
#define LINUX_TRACING_UPROBES_RETURN_ADDRESS_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_join.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>

#include <stack>
#include <vector>

#include "LibunwindstackMaps.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

// Keeps a stack, for every thread, of the return addresses at the top of the stack when dynamically
// instrumented functions are entered (e.g., when uprobes are hit), before they are hijacked to
// record the exits (e.g., by uretprobes). Patches them into samples so that unwinding can continue
// past dynamically instrumented functions.
class UprobesReturnAddressManager {
 public:
  explicit UprobesReturnAddressManager(
      UserSpaceInstrumentationAddresses* user_space_instrumentation_addresses)
      : user_space_instrumentation_addresses_{user_space_instrumentation_addresses} {}
  virtual ~UprobesReturnAddressManager() = default;

  UprobesReturnAddressManager(const UprobesReturnAddressManager&) = delete;
  UprobesReturnAddressManager& operator=(const UprobesReturnAddressManager&) = delete;

  UprobesReturnAddressManager(UprobesReturnAddressManager&&) = default;
  UprobesReturnAddressManager& operator=(UprobesReturnAddressManager&&) = default;

  virtual void ProcessFunctionEntry(pid_t tid, uint64_t stack_pointer, uint64_t return_address) {
    std::vector<OpenFunction>& stack_of_open_functions = tid_to_stack_of_open_functions_[tid];
    stack_of_open_functions.emplace_back(stack_pointer, return_address);
  }

  virtual void ProcessFunctionExit(pid_t tid) {
    if (!tid_to_stack_of_open_functions_.contains(tid)) {
      return;
    }

    std::vector<OpenFunction>& stack_of_open_functions = tid_to_stack_of_open_functions_.at(tid);
    ORBIT_CHECK(!stack_of_open_functions.empty());
    stack_of_open_functions.pop_back();
    if (stack_of_open_functions.empty()) {
      tid_to_stack_of_open_functions_.erase(tid);
    }
  }

  virtual void PatchSample(pid_t tid, uint64_t stack_pointer, void* stack_data,
                           uint64_t stack_size) {
    if (!tid_to_stack_of_open_functions_.contains(tid)) {
      return;
    }

    std::vector<OpenFunction>& stack_of_open_functions = tid_to_stack_of_open_functions_.at(tid);
    ORBIT_CHECK(!stack_of_open_functions.empty());

    // Apply saved return addresses in reverse order, from the last called function. In case two
    // return addresses are hijacked at the same stack pointer (e.g., in case of tail-call
    // optimization), this results in the correct original return address to end up in the patched
    // stack.
    for (auto it = stack_of_open_functions.rbegin(); it != stack_of_open_functions.rend(); it++) {
      const OpenFunction& open_function = *it;
      if (open_function.stack_pointer < stack_pointer) {
        continue;
      }
      const uint64_t& offset = open_function.stack_pointer - stack_pointer;
      if (offset >= stack_size) {
        continue;
      }

      memcpy(static_cast<uint8_t*>(stack_data) + offset, &open_function.return_address,
             sizeof(open_function.return_address));
    }
  }

  // In case of callchain sampling we don't have the complete stack to patch, but only the callchain
  // (as list of instruction pointers). In those, the address of a uretprobes or of a user space
  // instrumentation return trampoline occurs in place of the caller of an instrumented function.
  // This function patches the callchain, using the maps information to identify instruction
  // pointers of uretprobe code and using user_space_instrumentation_addresses_ to identify a user
  // space instrumentation return trampoline. The affected frames are replaced with the return
  // addresses saved by uprobes or user space instrumentation on function entry.
  virtual bool PatchCallchain(pid_t tid, uint64_t* callchain, uint64_t callchain_size,
                              LibunwindstackMaps* maps) {
    ORBIT_CHECK(callchain_size > 0);
    ORBIT_CHECK(callchain != nullptr);
    ORBIT_CHECK(maps != nullptr);

    std::vector<uint64_t> frames_to_patch;
    for (uint64_t i = 0; i < callchain_size; i++) {
      const uint64_t ip = callchain[i];

      if (user_space_instrumentation_addresses_ != nullptr &&
          user_space_instrumentation_addresses_->IsInReturnTrampoline(ip)) {
        frames_to_patch.push_back(i);
        continue;
      }

      std::shared_ptr<unwindstack::MapInfo> map_info = maps->Find(ip);
      if (map_info != nullptr && map_info->name() == "[uprobes]") {
        frames_to_patch.push_back(i);
      }
    }

    if (!tid_to_stack_of_open_functions_.contains(tid)) {
      // If there are no open dynamically instrumented function, but the callchain needs to be
      // patched, we need to discard the sample.
      // There are two situations where this may happen:
      //  1. At the beginning of a capture, where we missed the first entries into functions (e.g.,
      //     some uprobes);
      //  2. When some events are lost or processed out of order.
      if (!frames_to_patch.empty()) {
        ORBIT_ERROR(
            "Discarding sample in a dynamically instrumented function as all information is "
            "missing (tid=%d)",
            tid);
        return false;
      }
      return true;
    }

    std::vector<OpenFunction>& stack_of_open_functions = tid_to_stack_of_open_functions_.at(tid);
    ORBIT_CHECK(!stack_of_open_functions.empty());

    size_t num_unique_open_functions = 0;
    uint64_t prev_open_function_stack_pointer = -1;
    for (const OpenFunction& open_function : stack_of_open_functions) {
      if (open_function.stack_pointer != prev_open_function_stack_pointer) {
        num_unique_open_functions++;
      }
      prev_open_function_stack_pointer = open_function.stack_pointer;
    }

    // In case we have fewer open functions (with correct return address) than frames to be patched,
    // we need to discard this sample.
    // There are two situations where this may happen:
    //  1. At the beginning of a capture, where we missed the first entries into functions (e.g.,
    //     some uprobes);
    //  2. When some events are lost or processed out of order.
    // This is the same situation as above, but we have at least some open dynamically instrumented
    // functions.
    if (num_unique_open_functions < frames_to_patch.size()) {
      ORBIT_ERROR(
          "Discarding sample in a dynamically instrumented function as some information is "
          "missing (tid=%d)",
          tid);
      return false;
    }
    // In cases of lost events, or out of order processing, there might be wrong open dynamically
    // instrumented functions. So we need to discard the event.
    if (num_unique_open_functions > frames_to_patch.size() + 1) {
      ORBIT_ERROR(
          "Discarding sample in a dynamically instrumented function as some information is "
          "incorrect (tid=%d)",
          tid);
      return false;
    }

    // Process frames from the outermost to the innermost.
    auto frames_to_patch_it = frames_to_patch.rbegin();
    size_t num_open_functions = stack_of_open_functions.size();

    // There are two situations where this may true:
    //  1. At the very end of an instrumented function, where the return address was already
    //     restored.
    //  2. At the very beginning of an instrumented function, where the return address was not yet
    //     overridden.
    // In any case, dynamic instrumentation (e.g., uprobes) has not overridden the return address.
    // We do not need to patch the effect of dynamic instrumentation for this frame and can move
    // forward.
    bool skip_last_open_function = num_unique_open_functions == frames_to_patch.size() + 1;

    // On tail-call optimization, when instrumenting the caller and the callee, the correct
    // callstack will only contain the callee.
    // However, there are two open functions (with the same stack pointer), where the first one (the
    // caller's) contains the correct return address.
    prev_open_function_stack_pointer = -1;
    size_t unique_open_functions_so_far = 0;
    for (size_t open_function_i = 0; open_function_i < num_open_functions; open_function_i++) {
      // If the innermost frame does not need to be patched (see above), we are done and can skip
      // the last dynamically instrumented function.
      if (skip_last_open_function &&
          unique_open_functions_so_far + 1 == num_unique_open_functions) {
        break;
      }
      const OpenFunction& open_function = stack_of_open_functions[open_function_i];
      // In tail-call case, we have already processed the open function with the correct return
      // address and are done with that frame.
      if (open_function.stack_pointer == prev_open_function_stack_pointer) {
        continue;
      }
      prev_open_function_stack_pointer = open_function.stack_pointer;
      unique_open_functions_so_far++;

      uint64_t frame_to_patch = *frames_to_patch_it;
      callchain[frame_to_patch] = open_function.return_address;
      frames_to_patch_it++;
    }
    ORBIT_CHECK(frames_to_patch_it == frames_to_patch.rend());
    return true;
  }

 private:
  struct OpenFunction {
    OpenFunction(uint64_t stack_pointer, uint64_t return_address)
        : stack_pointer{stack_pointer}, return_address{return_address} {}
    uint64_t stack_pointer;
    uint64_t return_address;
  };

  absl::flat_hash_map<pid_t, std::vector<OpenFunction>> tid_to_stack_of_open_functions_{};

  UserSpaceInstrumentationAddresses* user_space_instrumentation_addresses_;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_UPROBES_RETURN_ADDRESS_MANAGER_H_
