#ifndef ORBIT_LINUX_TRACING_UPROBES_RETURN_ADDRESS_MANAGER_H_
#define ORBIT_LINUX_TRACING_UPROBES_RETURN_ADDRESS_MANAGER_H_

#include <OrbitBase/Logging.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>

#include <stack>
#include <vector>

#include "absl/container/flat_hash_map.h"

// Keeps a stack, for every thread, of the return addresses at the top of the
// stack when uprobes are hit, before they are hijacked by uretprobes. Patches
// them into samples so that unwinding can continue past
// dynamically-instrumented functions.
class UprobesReturnAddressManager {
 public:
  UprobesReturnAddressManager() = default;

  UprobesReturnAddressManager(const UprobesReturnAddressManager&) = delete;
  UprobesReturnAddressManager& operator=(const UprobesReturnAddressManager&) =
      delete;

  UprobesReturnAddressManager(UprobesReturnAddressManager&&) = default;
  UprobesReturnAddressManager& operator=(UprobesReturnAddressManager&&) =
      default;

  void ProcessUprobes(pid_t tid, uint64_t stack_pointer,
                      uint64_t return_address) {
    auto& tid_uprobes_stack = tid_uprobes_stacks_[tid];
    tid_uprobes_stack.emplace_back(stack_pointer, return_address);
  }

  void PatchSample(pid_t tid, uint64_t stack_pointer, char* stack_data,
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
    for (auto it = tid_uprobes_stack.rbegin(); it != tid_uprobes_stack.rend();
         it++) {
      const OpenUprobes& uprobes = *it;
      if (uprobes.stack_pointer < stack_pointer) {
        continue;
      }
      const uint64_t& offset = uprobes.stack_pointer - stack_pointer;
      if (offset >= stack_size) {
        continue;
      }

      *reinterpret_cast<uint64_t*>(&stack_data[offset]) =
          uprobes.return_address;
    }
  }

  void PatchCallchain(pid_t tid, uint64_t* callchain, uint64_t nr,
                      unwindstack::Maps* maps) {
    if (!tid_uprobes_stacks_.contains(tid)) {
      return;
    }

    auto& tid_uprobes_stack = tid_uprobes_stacks_.at(tid);
    CHECK(!tid_uprobes_stack.empty());

    // TODO(kuebler): What about tail-call optimization, where two uretprobes
    //  hijacked an address at the same stack pointer.
    auto uprobes_it = tid_uprobes_stack.rbegin();
    for (uint64_t i = 0; i < nr; i++) {
      uint64_t ip = callchain[i];
      unwindstack::MapInfo* map_info = maps->Find(ip);

      // Only patch Broken IPs
      if (map_info == nullptr || map_info->name != "[uprobes]") {
        continue;
      }

      const OpenUprobes& uprobes = *uprobes_it;
      callchain[i] = uprobes.return_address;
      uprobes_it++;

      // There can't be more frames to patch as uprobes.
      if (uprobes_it == tid_uprobes_stack.rend()) {
        break;
      }
    }
  }

  void ProcessUretprobes(pid_t tid) {
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

#endif  // ORBIT_LINUX_TRACING_UPROBES_RETURN_ADDRESS_MANAGER_H_
