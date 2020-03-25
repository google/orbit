#ifndef ORBIT_CORE_LINUX_TRACING_HANDLER_H_
#define ORBIT_CORE_LINUX_TRACING_HANDLER_H_

#include <OrbitLinuxTracing/Tracer.h>
#include <OrbitLinuxTracing/TracerListener.h>

#include "ContextSwitch.h"
#include "LinuxCallstackEvent.h"
#include "LinuxTracingSession.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "ScopeTimer.h"
#include "absl/synchronization/mutex.h"

class LinuxTracingHandler : LinuxTracing::TracerListener {
 public:
  static constexpr double DEFAULT_SAMPLING_FREQUENCY = 1000.0;

  LinuxTracingHandler(SamplingProfiler* sampling_profiler,
                      LinuxTracingSession* session, Process* target_process,
                      std::map<uint64_t, Function*>* selected_function_map,
                      uint64_t* num_context_switches)
      : sampling_profiler_(sampling_profiler),
        session_(session),
        target_process_(target_process),
        selected_function_map_(selected_function_map),
        num_context_switches_(num_context_switches) {}

  ~LinuxTracingHandler() override = default;
  LinuxTracingHandler(const LinuxTracingHandler&) = delete;
  LinuxTracingHandler& operator=(const LinuxTracingHandler&) = delete;
  LinuxTracingHandler(LinuxTracingHandler&&) = default;
  LinuxTracingHandler& operator=(LinuxTracingHandler&&) = default;

  void Start();

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
  void ProcessCallstackEvent(LinuxCallstackEvent&& event);

  SamplingProfiler* sampling_profiler_;
  LinuxTracingSession* session_;
  Process* target_process_;
  std::map<uint64_t, Function*>* selected_function_map_;
  uint64_t* num_context_switches_;

  std::unique_ptr<LinuxTracing::Tracer> tracer_;

  pid_t TimelineToThreadId(std::string timeline);
  absl::flat_hash_map<std::string, pid_t> timeline_to_thread_id_;
  // TODO: This is a hack to reuse thread tracks in the UI to show GPU events.
  // This needs to be fixed.
  pid_t current_timeline_thread_id = 100000;
};

#endif  // ORBIT_CORE_LINUX_TRACING_HANDLER_H_
