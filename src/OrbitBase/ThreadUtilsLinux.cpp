// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

// On Linux, "the thread name is a meaningful C language
// string, whose length is restricted to 16 characters,
// including the terminating null byte ('\0')".
static constexpr size_t kMaxThreadNameLength = 16;

static constexpr int32_t kInvalidLinuxThreadId = -1;
static constexpr int32_t kInvalidLinuxProcessId = -1;

static constexpr uint32_t kIntMax = static_cast<uint32_t>(INT_MAX);

uint32_t GetCurrentThreadId() { return FromNativeThreadId(GetCurrentThreadIdNative()); }

uint32_t GetCurrentProcessId() { return FromNativeProcessId(GetCurrentProcessIdNative()); }

bool IsValidThreadId(uint32_t tid) { return tid <= kIntMax && tid != kInvalidThreadId; }

bool IsValidProcessId(uint32_t pid) { return pid <= kIntMax && pid != kInvalidProcessId; }

pid_t GetCurrentThreadIdNative() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}

pid_t GetCurrentProcessIdNative() {
  static pid_t current_pid = getpid();
  return current_pid;
}

uint32_t FromNativeThreadId(pid_t tid) {
  ORBIT_CHECK(tid == kInvalidLinuxThreadId || tid >= 0);
  return static_cast<uint32_t>(tid);
}

uint32_t FromNativeProcessId(pid_t pid) {
  ORBIT_CHECK(pid == kInvalidLinuxProcessId || pid >= 0);
  return static_cast<uint32_t>(pid);
}

pid_t ToNativeThreadId(uint32_t tid) {
  ORBIT_CHECK(tid <= kIntMax || tid == kInvalidThreadId);
  return static_cast<pid_t>(tid);
}

pid_t ToNativeProcessId(uint32_t pid) {
  ORBIT_CHECK(pid <= kIntMax || pid == kInvalidProcessId);
  return static_cast<pid_t>(pid);
}

std::string GetThreadName(uint32_t tid) { return GetThreadNameNative(ToNativeThreadId(tid)); }

std::string GetThreadNameNative(pid_t tid) {
  std::string comm_filename = absl::StrFormat("/proc/%d/comm", tid);
  ErrorMessageOr<std::string> comm_content = ReadFileToString(comm_filename);
  if (!comm_content.has_value()) {
    ORBIT_ERROR("Getting thread name for tid %d: %s", tid, comm_content.error().message());
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
    ORBIT_ERROR("Setting thread name for tid %d. Error %d", GetCurrentThreadIdNative(), result);
  }
}

}  // namespace orbit_base
