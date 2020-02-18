//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#ifndef ORBIT_CORE_LINUX_EVENT_TRACER_H_
#define ORBIT_CORE_LINUX_EVENT_TRACER_H_

#include <atomic>
#include <functional>
#include <thread>
#include <utility>

#include "LibunwindstackUnwinder.h"
#include "OrbitFunction.h"
#include "PrintVar.h"

class LinuxEventTracerThread {
 public:
  LinuxEventTracerThread(pid_t pid, double sampling_frequency,
                         std::vector<Function*> instrumented_functions)
      : pid_(pid),
        sampling_frequency_(sampling_frequency),
        instrumented_functions_(std::move(instrumented_functions)) {}

  LinuxEventTracerThread(const LinuxEventTracerThread&) = delete;
  LinuxEventTracerThread& operator=(const LinuxEventTracerThread&) = delete;
  LinuxEventTracerThread(LinuxEventTracerThread&&) = default;
  LinuxEventTracerThread& operator=(LinuxEventTracerThread&&) = default;

  void Run(const std::shared_ptr<std::atomic<bool>>& exit_requested);

 private:
  // Number of records to read consecutively from a perf_event_open ring buffer
  // before switching to another one.
  static constexpr int32_t ROUND_ROBIN_BATCH_SIZE = 5;

  pid_t pid_;
  double sampling_frequency_;
  std::vector<Function*> instrumented_functions_;

  uint64_t sampling_period_ns_ = 1'000'000;
  int32_t num_cpus_ = 0;

  bool ComputeSamplingPeriodNs();
  void LoadNumCpus();
};

class LinuxEventTracer {
 public:
  static constexpr double DEFAULT_SAMPLING_FREQUENCY = 1000.0;

  LinuxEventTracer(pid_t pid, double sampling_frequency,
                   std::vector<Function*> instrumented_functions)
      : pid_{pid},
        sampling_frequency_{sampling_frequency},
        instrumented_functions_{std::move(instrumented_functions)} {}

  explicit LinuxEventTracer(pid_t pid)
      : LinuxEventTracer{pid, DEFAULT_SAMPLING_FREQUENCY, {}} {}

  LinuxEventTracer(pid_t pid, double sampling_frequency)
      : LinuxEventTracer{pid, sampling_frequency, {}} {}

  LinuxEventTracer(pid_t pid, std::vector<Function*> instrumented_functions)
      : LinuxEventTracer{pid, DEFAULT_SAMPLING_FREQUENCY,
                         std::move(instrumented_functions)} {}

  LinuxEventTracer(const LinuxEventTracer&) = delete;
  LinuxEventTracer& operator=(const LinuxEventTracer&) = delete;
  LinuxEventTracer(LinuxEventTracer&&) = default;
  LinuxEventTracer& operator=(LinuxEventTracer&&) = default;

  void SetInstrumentedFunctions(std::vector<Function*> instrumented_functions) {
    instrumented_functions_ = std::move(instrumented_functions);
  }

  void Start() {
    PRINT_FUNC;
    *exit_requested_ = false;
    thread_ = std::make_shared<std::thread>(
        &LinuxEventTracer::Run, pid_, sampling_frequency_,
        instrumented_functions_, exit_requested_);
    thread_->detach();
  }

  void Stop() { *exit_requested_ = true; }

 private:
  pid_t pid_;
  double sampling_frequency_;
  std::vector<Function*> instrumented_functions_;

  // exit_requested_ must outlive this object because it is used by thread_.
  // The control block of shared_ptr is thread safe (i.e., reference counting
  // and pointee's lifetime management are atomic and thread safe).
  std::shared_ptr<std::atomic<bool>> exit_requested_ =
      std::make_unique<std::atomic<bool>>(true);
  std::shared_ptr<std::thread> thread_;

  static void Run(pid_t pid, double sampling_frequency,
                  const std::vector<Function*>& instrumented_functions,
                  const std::shared_ptr<std::atomic<bool>>& exit_requested) {
    LinuxEventTracerThread{pid, sampling_frequency, instrumented_functions}.Run(
        exit_requested);
  }
};

#endif  // ORBIT_CORE_LINUX_EVENT_TRACER_H_
