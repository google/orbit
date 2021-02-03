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
#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

// On Linux, "the thread name is a meaningful C language
// string, whose length is restricted to 16 characters,
// including the terminating null byte ('\0')".
static constexpr size_t kMaxThreadNameLength = 16;

pid_t GetCurrentThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}

std::string GetThreadName(pid_t tid) {
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

void SetCurrentThreadName(const std::string& thread_name) {
  std::string name = thread_name;
  if (name.length() >= kMaxThreadNameLength) {
    name[kMaxThreadNameLength - 1] = '\0';
  }
  int result = pthread_setname_np(pthread_self(), name.data());
  if (result != 0) {
    ERROR("Setting thread name for tid %d. Error %d", GetCurrentThreadId(), result);
  }
}

pid_t GetCurrentProcessId() { return getpid(); }

}  // namespace orbit_base
