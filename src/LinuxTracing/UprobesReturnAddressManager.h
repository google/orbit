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

#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

// Keeps a stack, for every thread, of the return addresses at the top of the
// stack when uprobes are hit, before they are hijacked by uretprobes. Patches
// them into samples so that unwinding can continue past
// dynamically-instrumented functions.
class UprobesReturnAddressManager {
 public:
  UprobesReturnAddressManager() = default;

  UprobesReturnAddressManager(const UprobesReturnAddressManager&) = delete;
  UprobesReturnAddressManager& operator=(const UprobesReturnAddressManager&) = delete;

  UprobesReturnAddressManager(UprobesReturnAddressManager&&) = default;
  UprobesReturnAddressManager& operator=(UprobesReturnAddressManager&&) = default;

  virtual void ProcessUprobes(pid_t tid, uint64_t stack_pointer, uint64_t return_address) {
    auto& tid_uprobes_stack = tid_uprobes_stacks_[tid];
    tid_uprobes_stack.emplace_back(stack_pointer, return_address);
  }

  virtual void PatchSample(pid_t tid, uint64_t stack_pointer, void* stack_data,
                           uint64_t stack_size) {
    if (!tid_uprobes_stacks_.contains(tid)) {
      return;
    }

    auto& tid_uprobes_stack = tid_uprobes_stacks_.at(tid);
    CHECK(!tid_uprobes_stack.empty());

    // Apply saved return addresses in reverse order, from the last called
    // function. In case two uretprobes hijacked an address at the same stack
    // pointer (e.g., in case of tail-call optimization), this results in the
    // correct original return address to end up in the patched stack.
    for (auto it = tid_uprobes_stack.rbegin(); it != tid_uprobes_stack.rend(); it++) {
      const OpenUprobes& uprobes = *it;
      if (uprobes.stack_pointer < stack_pointer) {
        continue;
      }
      const uint64_t& offset = uprobes.stack_pointer - stack_pointer;
      if (offset >= stack_size) {
        continue;
      }

      memcpy(static_cast<uint8_t*>(stack_data) + offset, &uprobes.return_address,
             sizeof(uprobes.return_address));
    }
  }

  // In case of callchain sampling we don't have the complete stack to patch,
  // but only the callchain (as list of instruction pointers). In those,
  // a uprobe address occurs in place of the caller of an instrumented function.
  // This function patches the callchain, using the maps information to identify
  // instruction pointers of uprobe code and using the return address saved in
  // the uprobes.
  virtual bool PatchCallchain(pid_t tid, uint64_t* callchain, uint64_t callchain_size,
                              LibunwindstackMaps* maps) {
    CHECK(callchain_size > 0);
    CHECK(callchain != nullptr);
    CHECK(maps != nullptr);

    std::vector<uint64_t> frames_to_patch;
    for (uint64_t i = 0; i < callchain_size; i++) {
      uint64_t ip = callchain[i];
      unwindstack::MapInfo* map_info = maps->Find(ip);

      if (map_info == nullptr || map_info->name() != "[uprobes]") {
        continue;
      }

      frames_to_patch.push_back(i);
    }

    if (!tid_uprobes_stacks_.contains(tid)) {
      // If there are no uprobes, but the callchain needs to be patched, we need
      // to discard the sample.
      // There are two situations where this may happen:
      //  1. At the beginning of a capture, where we missed the first uprobes;
      //  2. When some events are lost or processed out of order.
      if (!frames_to_patch.empty()) {
        ERROR("Discarding sample in a uprobe as uprobe records are missing.");
        return false;
      }
      return true;
    }

    auto& tid_uprobes_stack = tid_uprobes_stacks_.at(tid);
    CHECK(!tid_uprobes_stack.empty());

    size_t num_unique_uprobes = 0;
    uint64_t prev_uprobe_stack_pointer = -1;
    for (const auto& uprobe : tid_uprobes_stack) {
      if (uprobe.stack_pointer != prev_uprobe_stack_pointer) {
        num_unique_uprobes++;
      }
      prev_uprobe_stack_pointer = uprobe.stack_pointer;
    }

    // In case we have less uprobes (with correct return address) than frames
    // to be patched, we need to discard this sample.
    // There are two situations where this may happen:
    //  1. At the beginning of a capture, where we missed the first uprobes
    //  2. When some events are lost or processed out of order.
    // This is the same situation as above, but we have at least some uprobe
    // records.
    if (num_unique_uprobes < frames_to_patch.size()) {
      ERROR("Discarding sample in a uprobe as some uprobe records are missing.");
      return false;
    }
    // In cases of lost events, or out of order processing, there might be wrong
    // uprobes. So we need to discard the event. In general we should be fast
    // enough, such that this does not happen.
    if (num_unique_uprobes > frames_to_patch.size() + 1) {
      ERROR("Discarding sample in a uprobe as uprobe records are incorrect.");
      return false;
    }

    // Process frames from the outermost to the innermost.
    auto frames_to_patch_it = frames_to_patch.rbegin();
    size_t uprobes_size = tid_uprobes_stack.size();

    // There are two situations where this may true:
    //  1. At the very end of an instrumented function, where the return
    //   address was already restored.
    //  2. At the very beginning of an instrumented function, where the return
    //   address was not yet overridden.
    // In any case, the uprobe(s) have not overridden the return address.
    // We do not need to patch the effect of this uprobe and can move forward.
    bool skip_last_uprobes = num_unique_uprobes == frames_to_patch.size() + 1;

    // On tail-call optimization, when instrumenting the caller and the callee,
    // the correct callstack will only contain the callee.
    // However, there are two uprobe records (with the same stack pointer),
    // where the first one (the caller's) contains the correct return address.
    prev_uprobe_stack_pointer = -1;
    size_t unique_uprobes_so_far = 0;
    for (size_t uprobe_i = 0; uprobe_i < uprobes_size; uprobe_i++) {
      // If the innermost frame does not need to be patched (see above), we are
      // done and can skip that last uprobes.
      if (skip_last_uprobes && unique_uprobes_so_far + 1 == num_unique_uprobes) {
        break;
      }
      const OpenUprobes& uprobe = tid_uprobes_stack[uprobe_i];
      // In tail-call case, we already have process the uprobe with the correct
      // return address and are done with that frame.
      if (uprobe.stack_pointer == prev_uprobe_stack_pointer) {
        continue;
      }
      prev_uprobe_stack_pointer = uprobe.stack_pointer;
      unique_uprobes_so_far++;

      uint64_t frame_to_patch = *frames_to_patch_it;
      callchain[frame_to_patch] = uprobe.return_address;
      frames_to_patch_it++;
    }
    CHECK(frames_to_patch_it == frames_to_patch.rend());
    return true;
  }

  virtual void ProcessUretprobes(pid_t tid) {
    if (!tid_uprobes_stacks_.contains(tid)) {
      return;
    }

    auto& tid_uprobes_stack = tid_uprobes_stacks_.at(tid);
    CHECK(!tid_uprobes_stack.empty());
    tid_uprobes_stack.pop_back();
    if (tid_uprobes_stack.empty()) {
      tid_uprobes_stacks_.erase(tid);
    }
  }

 private:
  struct OpenUprobes {
    OpenUprobes(uint64_t stack_pointer, uint64_t return_address)
        : stack_pointer{stack_pointer}, return_address{return_address} {}
    uint64_t stack_pointer;
    uint64_t return_address;
  };

  absl::flat_hash_map<pid_t, std::vector<OpenUprobes>> tid_uprobes_stacks_{};
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_UPROBES_RETURN_ADDRESS_MANAGER_H_
