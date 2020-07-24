// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_LINUX_TRACING_BUFFER_H_
#define ORBIT_CORE_LINUX_TRACING_BUFFER_H_

#include "ContextSwitch.h"
#include "EventBuffer.h"
#include "KeyAndString.h"
#include "LinuxAddressInfo.h"
#include "LinuxCallstackEvent.h"
#include "ScopeTimer.h"
#include "StringManager.h"
#include "TidAndThreadName.h"
#include "absl/synchronization/mutex.h"

// This class buffers tracing data to be sent to the client
// and provides thread-safe access and record functions.
class LinuxTracingBuffer {
 public:
  LinuxTracingBuffer() = default;

  LinuxTracingBuffer(const LinuxTracingBuffer&) = delete;
  LinuxTracingBuffer& operator=(const LinuxTracingBuffer&) = delete;
  LinuxTracingBuffer(LinuxTracingBuffer&&) = delete;
  LinuxTracingBuffer& operator=(LinuxTracingBuffer&&) = delete;

  void RecordTimer(Timer&& timer);
  void RecordCallstack(LinuxCallstackEvent&& callstack);
  void RecordHashedCallstack(
      orbit_client_protos::CallstackEvent&& hashed_callstack);
  void RecordAddressInfo(LinuxAddressInfo&& address_info);
  void RecordKeyAndString(KeyAndString&& key_and_string);
  void RecordKeyAndString(uint64_t key, std::string str);
  void RecordThreadName(TidAndThreadName&& tid_and_name);
  void RecordThreadName(int32_t tid, std::string name);

  // These move the content of the corresponding buffer to the output vector.
  // They return true if the buffer was not empty.
  bool ReadAllTimers(std::vector<Timer>* out_buffer);
  bool ReadAllCallstacks(std::vector<LinuxCallstackEvent>* out_buffer);
  bool ReadAllHashedCallstacks(
      std::vector<orbit_client_protos::CallstackEvent>* out_buffer);
  bool ReadAllAddressInfos(std::vector<LinuxAddressInfo>* out_buffer);
  bool ReadAllKeysAndStrings(std::vector<KeyAndString>* out_buffer);
  bool ReadAllThreadNames(std::vector<TidAndThreadName>* out_buffer);

  void Reset();

 private:
  // Buffering data to send large messages instead of small ones.
  absl::Mutex timer_buffer_mutex_;
  std::vector<Timer> timer_buffer_;

  absl::Mutex callstack_buffer_mutex_;
  std::vector<LinuxCallstackEvent> callstack_buffer_;

  absl::Mutex hashed_callstack_buffer_mutex_;
  std::vector<orbit_client_protos::CallstackEvent> hashed_callstack_buffer_;

  absl::Mutex address_info_buffer_mutex_;
  std::vector<LinuxAddressInfo> address_info_buffer_;

  absl::Mutex key_and_string_buffer_mutex_;
  std::vector<KeyAndString> key_and_string_buffer_;

  absl::Mutex thread_name_buffer_mutex_;
  std::vector<TidAndThreadName> thread_name_buffer_;
};

#endif  // ORBIT_CORE_LINUX_TRACING_BUFFER_H_
