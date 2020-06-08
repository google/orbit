// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracingBuffer.h"

#include <utility>

#include "KeyAndString.h"
#include "TcpServer.h"

void LinuxTracingBuffer::RecordTimer(Timer&& timer) {
  absl::MutexLock lock(&timer_buffer_mutex_);
  timer_buffer_.emplace_back(std::move(timer));
}

void LinuxTracingBuffer::RecordCallstack(LinuxCallstackEvent&& callstack) {
  absl::MutexLock lock(&callstack_buffer_mutex_);
  callstack_buffer_.emplace_back(std::move(callstack));
}

void LinuxTracingBuffer::RecordHashedCallstack(
    CallstackEvent&& hashed_callstack) {
  absl::MutexLock lock(&hashed_callstack_buffer_mutex_);
  hashed_callstack_buffer_.emplace_back(std::move(hashed_callstack));
}

void LinuxTracingBuffer::RecordAddressInfo(LinuxAddressInfo&& address_info) {
  absl::MutexLock lock(&address_info_buffer_mutex_);
  address_info_buffer_.emplace_back(std::move(address_info));
}

void LinuxTracingBuffer::RecordKeyAndString(KeyAndString&& key_and_string) {
  absl::MutexLock lock(&key_and_string_buffer_mutex_);
  key_and_string_buffer_.emplace_back(std::move(key_and_string));
}

void LinuxTracingBuffer::RecordKeyAndString(uint64_t key,
                                            const std::string& str) {
  RecordKeyAndString({key, str});
}

void LinuxTracingBuffer::RecordThreadName(TidAndThreadName&& tid_and_name) {
  absl::MutexLock lock(&thread_name_buffer_mutex_);
  thread_name_buffer_.emplace_back(std::move(tid_and_name));
}

void LinuxTracingBuffer::RecordThreadName(uint32_t tid,
                                          const std::string& name) {
  RecordThreadName({tid, name});
}

bool LinuxTracingBuffer::ReadAllTimers(std::vector<Timer>* out_buffer) {
  absl::MutexLock lock(&timer_buffer_mutex_);
  if (timer_buffer_.empty()) {
    return false;
  }

  *out_buffer = std::move(timer_buffer_);
  timer_buffer_.clear();
  return true;
}

bool LinuxTracingBuffer::ReadAllCallstacks(
    std::vector<LinuxCallstackEvent>* out_buffer) {
  absl::MutexLock lock(&callstack_buffer_mutex_);
  if (callstack_buffer_.empty()) {
    return false;
  }

  *out_buffer = std::move(callstack_buffer_);
  callstack_buffer_.clear();
  return true;
}

bool LinuxTracingBuffer::ReadAllHashedCallstacks(
    std::vector<CallstackEvent>* out_buffer) {
  absl::MutexLock lock(&hashed_callstack_buffer_mutex_);
  if (hashed_callstack_buffer_.empty()) {
    return false;
  }

  *out_buffer = std::move(hashed_callstack_buffer_);
  hashed_callstack_buffer_.clear();
  return true;
}

bool LinuxTracingBuffer::ReadAllAddressInfos(
    std::vector<LinuxAddressInfo>* out_buffer) {
  absl::MutexLock lock(&address_info_buffer_mutex_);
  if (address_info_buffer_.empty()) {
    return false;
  }

  *out_buffer = std::move(address_info_buffer_);
  address_info_buffer_.clear();
  return true;
}

bool LinuxTracingBuffer::ReadAllKeysAndStrings(
    std::vector<KeyAndString>* out_buffer) {
  absl::MutexLock lock(&key_and_string_buffer_mutex_);
  if (key_and_string_buffer_.empty()) {
    return false;
  }

  *out_buffer = std::move(key_and_string_buffer_);
  key_and_string_buffer_.clear();
  return true;
}

bool LinuxTracingBuffer::ReadAllThreadNames(
    std::vector<TidAndThreadName>* out_buffer) {
  absl::MutexLock lock(&thread_name_buffer_mutex_);
  if (thread_name_buffer_.empty()) {
    return false;
  }

  *out_buffer = std::move(thread_name_buffer_);
  thread_name_buffer_.clear();
  return true;
}

void LinuxTracingBuffer::Reset() {
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
  {
    absl::MutexLock lock(&address_info_buffer_mutex_);
    address_info_buffer_.clear();
  }
  {
    absl::MutexLock lock(&key_and_string_buffer_mutex_);
    key_and_string_buffer_.clear();
  }
  {
    absl::MutexLock lock(&thread_name_buffer_mutex_);
    thread_name_buffer_.clear();
  }
}
