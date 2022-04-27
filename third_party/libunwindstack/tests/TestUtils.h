/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LIBUNWINDSTACK_TESTS_TEST_UTILS_H
#define _LIBUNWINDSTACK_TESTS_TEST_UTILS_H

#include <errno.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace unwindstack {

class TestScopedPidReaper {
 public:
  TestScopedPidReaper(pid_t pid) : pid_(pid) {}
  ~TestScopedPidReaper() {
    kill(pid_, SIGKILL);
    waitpid(pid_, nullptr, 0);
  }

 private:
  pid_t pid_;
};

inline bool TestQuiescePid(pid_t pid) {
  siginfo_t si;
  // Wait for up to 10 seconds.
  for (size_t i = 0; i < 10000; i++) {
    if (ptrace(PTRACE_GETSIGINFO, pid, 0, &si) == 0) {
      return true;
    }
    if (errno != ESRCH) {
      if (errno == EINVAL) {
        // The process is in group-stop state, so try and kick the
        // process out of that state.
        if (ptrace(PTRACE_LISTEN, pid, 0, 0) == -1) {
          perror("ptrace listen failed.");
          return false;
        }
      } else {
        return false;
      }
    }
    usleep(1000);
  }
  return false;
}

inline bool TestAttach(pid_t pid) {
  // Wait up to 10 seconds to attach.
  for (size_t j = 0; j < 10000; j++) {
    if (ptrace(PTRACE_ATTACH, pid, 0, 0) == 0) {
      break;
    }
    if (errno == ESRCH) {
      usleep(1000);
      continue;
    }
    perror("Failed to attach.");
    return false;
  }

  if (TestQuiescePid(pid)) {
    return true;
  }

  if (ptrace(PTRACE_DETACH, pid, 0, 0) == -1) {
    perror("Failed to detach.");
  }
  return false;
}

inline bool TestDetach(pid_t pid) {
  if (ptrace(PTRACE_DETACH, pid, 0, 0) == -1) {
    perror("ptrace detach failed");
    return false;
  }
  return true;
}

void TestCheckForLeaks(void (*unwind_func)(void*), void* data);

void* GetTestLibHandle();

// TODO(b/148307629): Once we incorporate google benchmark library into
// GoogleTest, we can call benchmark::DoNotOptimize here instead.
template <class Tp>
static inline void DoNotOptimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_TESTS_TEST_UTILS_H
