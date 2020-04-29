#ifndef ORBIT_CORE_LINUX_TRACING_HANDLER_H_
#define ORBIT_CORE_LINUX_TRACING_HANDLER_H_

#include <OrbitLinuxTracing/Tracer.h>
#include <OrbitLinuxTracing/TracerListener.h>

#include "ContextSwitch.h"
#include "LinuxCallstackEvent.h"
#include "LinuxTracingBuffer.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "ScopeTimer.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"

class LinuxTracingHandler : LinuxTracing::TracerListener {
 public:
  explicit LinuxTracingHandler(LinuxTracingBuffer* tracing_buffer)
      : tracing_buffer_{tracing_buffer} {}

  ~LinuxTracingHandler() override = default;
  LinuxTracingHandler(const LinuxTracingHandler&) = delete;
  LinuxTracingHandler& operator=(const LinuxTracingHandler&) = delete;
  LinuxTracingHandler(LinuxTracingHandler&&) = delete;
  LinuxTracingHandler& operator=(LinuxTracingHandler&&) = delete;

  void Start(pid_t pid,
             const std::map<uint64_t, Function*>& selected_function_map);
  bool IsStarted();
  void Stop();

  void OnTid(pid_t tid) override;
  void OnContextSwitchIn(
      const LinuxTracing::ContextSwitchIn& context_switch_in) override;
  void OnContextSwitchOut(
      const LinuxTracing::ContextSwitchOut& context_switch_out) override;
  void OnCallstack(const LinuxTracing::Callstack& callstack) override;
  void OnFunctionCall(const LinuxTracing::FunctionCall& function_call) override;
  void OnGpuJob(const LinuxTracing::GpuJob& gpu_job) override;

 private:
  uint64_t ProcessStringAndGetKey(const std::string& string);

  LinuxTracingBuffer* tracing_buffer_;
  std::unique_ptr<LinuxTracing::Tracer> tracer_;

  absl::flat_hash_set<uint64_t> addresses_seen_;
  absl::Mutex addresses_seen_mutex_;
  absl::flat_hash_set<uint64_t> callstack_hashes_seen_;
  absl::Mutex callstack_hashes_seen_mutex_;
  StringManager string_manager_;

  // TODO: Fix this hack that reuses thread tracks in the UI to show GPU events.
  pid_t TimelineToThreadId(std::string_view timeline);
  absl::flat_hash_map<std::string, pid_t> timeline_to_thread_id_;
  pid_t current_timeline_thread_id_ = 1'000'000'000;
};

#endif  // ORBIT_CORE_LINUX_TRACING_HANDLER_H_
