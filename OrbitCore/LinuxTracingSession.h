#ifndef ORBIT_CORE_LINUX_TRACING_SESSION_H_
#define ORBIT_CORE_LINUX_TRACING_SESSION_H_

#include "ContextSwitch.h"
#include "EventBuffer.h"
#include "LinuxCallstackEvent.h"
#include "ScopeTimer.h"
#include "StringManager.h"
#include "TcpServer.h"

#include "absl/synchronization/mutex.h"

// This class stores information about tracing session
// and provides thread-safe access and record functions.
class LinuxTracingSession {
 public:
  explicit LinuxTracingSession(TcpServer* tcp_server);
  LinuxTracingSession(const LinuxTracingSession&) = delete;
  LinuxTracingSession& operator=(const LinuxTracingSession&) = delete;

  void RecordContextSwitch(ContextSwitch&& context_switch);
  void RecordTimer(Timer&& timer);
  void RecordCallstack(LinuxCallstackEvent&& event);
  void RecordHashedCallstack(CallstackEvent&& event);

  void SetStringManager(std::shared_ptr<StringManager> string_manager);
  void SendKeyAndString(uint64_t hash, const std::string& name);

  // These move the content of corresponding buffer to
  // the output vector. They return true if the buffer
  // is not empty.
  bool ReadAllContextSwitches(std::vector<ContextSwitch>* buffer);
  bool ReadAllTimers(std::vector<Timer>* buffer);
  bool ReadAllCallstacks(std::vector<LinuxCallstackEvent>* buffer);
  bool ReadAllHashedCallstacks(std::vector<CallstackEvent>* buffer);

  void Reset();

 private:
  // Buffering data to send large messages instead of small ones.
  absl::Mutex context_switch_buffer_mutex_;
  std::vector<ContextSwitch> context_switch_buffer_;

  absl::Mutex timer_buffer_mutex_;
  std::vector<Timer> timer_buffer_;

  absl::Mutex callstack_buffer_mutex_;
  std::vector<LinuxCallstackEvent> callstack_buffer_;

  absl::Mutex hashed_callstack_buffer_mutex_;
  std::vector<CallstackEvent> hashed_callstack_buffer_;

  TcpServer* tcp_server_;
  std::shared_ptr<StringManager> string_manager_;
};

#endif  // ORBIT_CORE_LINUX_TRACING_SESSION_H_
