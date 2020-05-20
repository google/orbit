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

  // In case of callchain sampling we don't have the complete stack to patch,
  // but only the callchain (as list of instruction pointers). In those,
  // a uprobe address occurs in place of the caller of an instrumented function.
  // This function patches the callchain, using the maps information to identify
  // instruction pointers of uprobe code and using the return address safed in
  // the uprobes.
  bool PatchCallchain(pid_t tid, uint64_t* callchain, uint64_t callchain_size,
                      unwindstack::Maps* maps) {
    if (!tid_uprobes_stacks_.contains(tid)) {
      return true;
    }

    auto& tid_uprobes_stack = tid_uprobes_stacks_.at(tid);
    CHECK(!tid_uprobes_stack.empty());

    std::vector<uint64_t> ips_to_patch;

    // TODO(kuebler): What about tail-call optimization, where two uretprobes
    //  hijacked an address at the same stack pointer.
    for (uint64_t i = 0; i < callchain_size; i++) {
      uint64_t ip = callchain[i];
      unwindstack::MapInfo* map_info = maps->Find(ip);

      if (map_info == nullptr || map_info->name != "[uprobes]") {
        continue;
      }

      ips_to_patch.push_back(i);
    }

    // In case we already used all uprobes, we need to discard this sample.
    // There are two situations where this may happen:
    //  1. At the beginning of a capture, where we missed the first uprobes
    //  2. When there are two many events, resulting in lost events or
    //   processing out of order.
    if (tid_uprobes_stack.size() < ips_to_patch.size()) {
      ERROR(
          "Discarding sample in an uprobe as some uprobe records are missing.");
      return false;
    }
    // In cases of lost events, or out of order processing, there might be wrong
    //  uprobes. So we need to discard the event. In general we should be fast
    //  enough, such that this does not happen.
    if (tid_uprobes_stack.size() > ips_to_patch.size() + 1) {
      ERROR("Discarding sample in an uprobe as uprobe records are incorrect.");
      return false;
    }

    auto uprobes_it = tid_uprobes_stack.rbegin();
    // There are two situations where this may happen:
    //  1. At the very end of an instrumented function, where the return
    //   address was already restored.
    //  2. At the very beginning of an instrumented function, where the return
    //   address was not yet overridden.
    // In any case, the inner most uprobe has not overridden the return address.
    // We do not need to patch the affect of this uprobe and can move forward.
    if (tid_uprobes_stack.size() == ips_to_patch.size() + 1) {
      uprobes_it++;
    }
    for (uint64_t i : ips_to_patch) {
      const OpenUprobes& uprobe = *uprobes_it;
      callchain[i] = uprobe.return_address;
      uprobes_it++;
    }

    CHECK(uprobes_it == tid_uprobes_stack.rend());
    return true;
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
