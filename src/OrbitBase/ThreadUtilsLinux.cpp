// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <pthread.h>
#include <syscall.h>
#include <time.h>
#include <unistd.h>

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

uint32_t GetCurrentThreadId() { return GetThreadIdFromNative(GetCurrentThreadIdNative()); }

uint32_t GetCurrentProcessId() { return GetProcessIdFromNative(GetCurrentProcessIdNative()); }

[[nodiscard]] std::string GetThreadName(uint32_t tid) {
  return GetThreadNameNative(GetNativeThreadId(tid));
}

// On Linux, "the thread name is a meaningful C language
// string, whose length is restricted to 16 characters,
// including the terminating null byte ('\0')".
static constexpr size_t kMaxThreadNameLength = 16;

pid_t GetCurrentThreadIdNative() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}

std::string GetThreadNameNative(pid_t tid) {
  std::string comm_filename = absl::StrFormat("/proc/%d/comm", tid);
  ErrorMessageOr<std::string> comm_content = ReadFileToString(comm_filename);
  if (!comm_content.has_value()) {
    ERROR("Getting thread name for tid %d: %s", tid, comm_content.error().message());
    return "";
  }
  if (!comm_content.value().empty() && comm_content.value().back() == '\n') {
    comm_content.value().pop_back();
  }
  return comm_content.value();
}

void SetCurrentThreadName(const char* thread_name) {
  std::string buf;
  if (strlen(thread_name) >= kMaxThreadNameLength) {
    buf.assign(thread_name, kMaxThreadNameLength - 1);
    thread_name = buf.c_str();
  }

  int result = pthread_setname_np(pthread_self(), thread_name);
  if (result != 0) {
    ERROR("Setting thread name for tid %d. Error %d", GetCurrentThreadIdNative(), result);
  }
}

pid_t GetCurrentProcessIdNative() { return getpid(); }

uint32_t GetThreadIdFromNative(pid_t tid) {
  if (tid == -1) {
    return orbit_base::kInvalidThreadId;
  }
  return static_cast<uint32_t>(tid);
}

uint32_t GetProcessIdFromNative(pid_t pid) {
  if (pid == -1) {
    return orbit_base::kInvalidProcessId;
  }
  return static_cast<uint32_t>(pid);
}

pid_t GetNativeThreadId(uint32_t tid) {
  if (tid == kInvalidThreadId) {
    return -1;
  }
  return static_cast<pid_t>(tid);
}

pid_t GetNativeProcessId(uint32_t pid) {
  if (pid == kInvalidProcessId) {
    return -1;
  }
  return static_cast<pid_t>(pid);
}
}  // namespace orbit_base
