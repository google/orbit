#ifndef ORBIT_LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_
#define ORBIT_LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_

#include <OrbitLinuxTracing/Events.h>
#include <OrbitLinuxTracing/TracerListener.h>

#include <stack>
#include <utility>

#include "LibunwindstackUnwinder.h"
#include "PerfEvent.h"
#include "PerfEventVisitor.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesReturnAddressManager.h"
#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

// UprobesUnwindingVisitor processes stack samples and uprobes/uretprobes
// records (as well as memory maps changes, to keep necessary unwinding
// information up-to-date), assuming they come in order. The reason for
// processing both in the same visitor is that, when entering a
// dynamically-instrumented function, the return address saved on the stack is
// hijacked by uretprobes. This causes unwinding of any (time-based) stack
// sample that falls inside such a function to stop at the first such function,
// with a frame in the [uprobes] map.
// To solve this, UprobesReturnAddressManager keeps a stack, for every thread,
// of the return addresses before they are hijacked, and patches them into the
// time-based stack samples. Such return addresses can be retrieved by getting
// the eight bytes at the top of the stack on hitting uprobes.
// TODO: Make this more robust to losing uprobes or uretprobes events, if this
//  is still observed. For example, pass the address of uretprobes and compare
//  it against the address of uprobes on the stack.

class UprobesUnwindingVisitor : public PerfEventVisitor {
 public:
  explicit UprobesUnwindingVisitor(const std::string& initial_maps)
      : current_maps_{LibunwindstackUnwinder::ParseMaps(initial_maps)} {}

  UprobesUnwindingVisitor(const UprobesUnwindingVisitor&) = delete;
  UprobesUnwindingVisitor& operator=(const UprobesUnwindingVisitor&) = delete;

  UprobesUnwindingVisitor(UprobesUnwindingVisitor&&) = default;
  UprobesUnwindingVisitor& operator=(UprobesUnwindingVisitor&&) = default;

  void SetListener(TracerListener* listener) { listener_ = listener; }

  void SetUnwindErrorsAndDiscardedSamplesCounters(
      std::shared_ptr<std::atomic<uint64_t>> unwind_error_counter,
      std::shared_ptr<std::atomic<uint64_t>>
          discarded_samples_in_uretprobes_counter) {
    unwind_error_counter_ = std::move(unwind_error_counter);
    discarded_samples_in_uretprobes_counter_ =
        std::move(discarded_samples_in_uretprobes_counter);
  }

  void visit(StackSamplePerfEvent* event) override;
  void visit(CallchainSamplePerfEvent* event) override;
  void visit(UprobesPerfEvent* event) override;
  void visit(UretprobesPerfEvent* event) override;
  void visit(MapsPerfEvent* event) override;

 private:
  UprobesFunctionCallManager function_call_manager_{};
  UprobesReturnAddressManager return_address_manager_{};
  std::unique_ptr<unwindstack::BufferMaps> current_maps_;
  LibunwindstackUnwinder unwinder_{};

  TracerListener* listener_ = nullptr;
  std::shared_ptr<std::atomic<uint64_t>> unwind_error_counter_ = nullptr;
  std::shared_ptr<std::atomic<uint64_t>>
      discarded_samples_in_uretprobes_counter_ = nullptr;

  static std::vector<CallstackFrame> CallstackFramesFromLibunwindstackFrames(
      const std::vector<unwindstack::FrameData>& libunwindstack_frames);

  static std::vector<CallstackFrame> CallstackFramesFromInstructionPointers(
      const uint64_t* frames, uint64_t size);

  absl::flat_hash_map<pid_t,
                      std::vector<std::tuple<uint64_t, uint64_t, uint32_t>>>
      uprobe_sps_ips_cpus_per_thread_{};
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_UPROBES_UNWINDING_VISITOR_H_
