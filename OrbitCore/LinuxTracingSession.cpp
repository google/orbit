#include "LinuxTracingSession.h"

#include <utility>

#include "KeyAndString.h"
#include "TcpServer.h"

LinuxTracingSession::LinuxTracingSession(TcpServer* tcp_server)
    : tcp_server_(tcp_server) {}

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

void LinuxTracingSession::SetStringManager(
    std::shared_ptr<StringManager> string_manager) {
  string_manager_ = string_manager;
}

void LinuxTracingSession::SendKeyAndString(uint64_t key,
                                           const std::string& str) {
  KeyAndString key_and_string;
  key_and_string.key = key;
  key_and_string.str = str;
  // TODO: This is not atomic, we might end up sending multiple keys
  // maybe change it to AddIfDoesNotExists()
  if (!string_manager_->Exists(key)) {
    std::string message_data = SerializeObjectBinary(key_and_string);
    // TODO: Remove networking from here.
    tcp_server_->Send(Msg_KeyAndString, message_data.c_str(),
                      message_data.size());
    string_manager_->Add(key, str);
  }
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
