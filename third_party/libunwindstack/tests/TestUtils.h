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

#pragma once

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void TestCheckForLeaks(void (*unwind_func)(void*), void* data);

void* GetTestLibHandle();

// TODO(b/148307629): Once we incorporate google benchmark library into
// GoogleTest, we can call benchmark::DoNotOptimize here instead.
template <class Tp>
static inline void DoNotOptimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

}  // namespace unwindstack
