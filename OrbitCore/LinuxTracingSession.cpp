#include "LinuxTracingSession.h"

#include <utility>

void LinuxTracingSession::RecordContextSwitch(ContextSwitch&& context_switch) {
  absl::MutexLock lock(&context_switch_buffer_mutex_);
  context_switch_buffer_.push_back(std::move(context_switch));
}

void LinuxTracingSession::RecordTimer(Timer&& timer) {
  absl::MutexLock lock(&timer_buffer_mutex_);
  timer_buffer_.push_back(std::move(timer));
}

void LinuxTracingSession::RecordCallstack(LinuxCallstackEvent&& event) {
  absl::MutexLock lock(&callstack_buffer_mutex_);
  callstack_buffer_.push_back(std::move(event));
}

void LinuxTracingSession::RecordHashedCallstack(
    CallstackEvent&& hashed_call_stack) {
  absl::MutexLock lock(&hashed_callstack_buffer_mutex_);
  hashed_callstack_buffer_.push_back(std::move(hashed_call_stack));
}

bool LinuxTracingSession::ReadAllContextSwitches(
    std::vector<ContextSwitch>* buffer) {
  absl::MutexLock lock(&context_switch_buffer_mutex_);
  if (context_switch_buffer_.empty()) {
    return false;
  }

  *buffer = std::move(context_switch_buffer_);
  context_switch_buffer_.clear();
  return true;
}

bool LinuxTracingSession::ReadAllTimers(std::vector<Timer>* buffer) {
  absl::MutexLock lock(&timer_buffer_mutex_);
  if (timer_buffer_.empty()) {
    return false;
  }

  *buffer = std::move(timer_buffer_);
  timer_buffer_.clear();
  return true;
}

bool LinuxTracingSession::ReadAllCallstacks(
    std::vector<LinuxCallstackEvent>* buffer) {
  absl::MutexLock lock(&callstack_buffer_mutex_);
  if (callstack_buffer_.empty()) {
    return false;
  }

  *buffer = std::move(callstack_buffer_);
  callstack_buffer_.clear();
  return true;
}

bool LinuxTracingSession::ReadAllHashedCallstacks(
    std::vector<CallstackEvent>* buffer) {
  absl::MutexLock lock(&hashed_callstack_buffer_mutex_);
  if (hashed_callstack_buffer_.empty()) {
    return false;
  }

  *buffer = std::move(hashed_callstack_buffer_);
  hashed_callstack_buffer_.clear();
  return true;
}

void LinuxTracingSession::Reset() {
  {
    absl::MutexLock lock(&context_switch_buffer_mutex_);
    context_switch_buffer_.clear();
  }

  {
    absl::MutexLock lock(&timer_buffer_mutex_);
    timer_buffer_.clear();
  }

  {
    absl::MutexLock lock(&callstack_buffer_mutex_);
    callstack_buffer_.clear();
  }

  {
    absl::MutexLock lock(&hashed_callstack_buffer_mutex_);
    hashed_callstack_buffer_.clear();
  }
}

