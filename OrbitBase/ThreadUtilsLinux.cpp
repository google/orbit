// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <OrbitBase/Logging.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

[[nodiscard]] thread_id_t GetCurrentThreadId() {
  thread_local thread_id_t current_tid = syscall(__NR_gettid);
  return current_tid;
}

std::string GetThreadName(thread_id_t tid) {
  constexpr size_t kMaxStringSize = 16;
  char thread_name[kMaxStringSize];
  int result = pthread_getname_np(pthread_self(), thread_name, kMaxStringSize);
  if (result != 0) {
    ERROR("Getting thread name for tid %d. Error %d", tid, result);
    return {};
  }
  return thread_name;
}

void SetCurrentThreadName(const std::string& thread_name) {
  // On Linux, "the thread name is a meaningful C language
  // string, whose length is restricted to 16 characters,
  // including the terminating null byte ('\0')".
  constexpr size_t kMaxStringSize = 16;
  std::string name = thread_name;
  if (name.length() >= kMaxStringSize) {
    name[kMaxStringSize - 1] = '\0';
  }
  int result = pthread_setname_np(pthread_self(), name.data());
  if (result != 0) {
    ERROR("Setting thread name for tid %d. Error %d", GetCurrentThreadId(), result);
  }
}

}  // namespace orbit_base
