// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_LINUX_TRACING_BUFFER_H_
#define ORBIT_CORE_LINUX_TRACING_BUFFER_H_

#include <gtest/gtest_prod.h>

#include "ContextSwitch.h"
#include "EventBuffer.h"
#include "KeyAndString.h"
#include "LinuxAddressInfo.h"
#include "LinuxCallstackEvent.h"
#include "ScopeTimer.h"
#include "StringManager.h"
#include "TcpServer.h"
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

  void RecordContextSwitch(ContextSwitch&& context_switch);
  void RecordTimer(Timer&& timer);
  void RecordCallstack(LinuxCallstackEvent&& callstack);
  void RecordHashedCallstack(CallstackEvent&& hashed_callstack);
  void RecordAddressInfo(LinuxAddressInfo&& address_info);
  void RecordKeyAndString(KeyAndString&& key_and_string);
  void RecordKeyAndString(uint64_t key, const std::string& str);
  void RecordThreadName(TidAndThreadName&& tid_and_name);
  void RecordThreadName(uint32_t tid, const std::string& name);

  // These move the content of the corresponding buffer to the output vector.
  // They return true if the buffer was not empty.
  bool ReadAllContextSwitches(std::vector<ContextSwitch>* out_buffer);
  bool ReadAllTimers(std::vector<Timer>* out_buffer);
  bool ReadAllCallstacks(std::vector<LinuxCallstackEvent>* out_buffer);
  bool ReadAllHashedCallstacks(std::vector<CallstackEvent>* out_buffer);
  bool ReadAllAddressInfos(std::vector<LinuxAddressInfo>* out_buffer);
  bool ReadAllKeysAndStrings(std::vector<KeyAndString>* out_buffer);
  bool ReadAllThreadNames(std::vector<TidAndThreadName>* out_buffer);

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

  absl::Mutex address_info_buffer_mutex_;
  std::vector<LinuxAddressInfo> address_info_buffer_;

  absl::Mutex key_and_string_buffer_mutex_;
  std::vector<KeyAndString> key_and_string_buffer_;

  absl::Mutex thread_name_buffer_mutex_;
  std::vector<TidAndThreadName> thread_name_buffer_;

  FRIEND_TEST(LinuxTracingBuffer, Empty);
  FRIEND_TEST(LinuxTracingBuffer, ContextSwitches);
  FRIEND_TEST(LinuxTracingBuffer, Timers);
  FRIEND_TEST(LinuxTracingBuffer, Callstacks);
  FRIEND_TEST(LinuxTracingBuffer, HashedCallstacks);
  FRIEND_TEST(LinuxTracingBuffer, AddressInfos);
  FRIEND_TEST(LinuxTracingBuffer, KeysAndStrings);
  FRIEND_TEST(LinuxTracingBuffer, ThreadNames);
  FRIEND_TEST(LinuxTracingBuffer, Reset);
};

#endif  // ORBIT_CORE_LINUX_TRACING_BUFFER_H_
