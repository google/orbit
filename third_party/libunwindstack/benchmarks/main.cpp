/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <benchmark/benchmark.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <android-base/strings.h>

#include <string>
#include <vector>

#if defined(__APPLE__)

// Darwin doesn't support this, so do nothing.
bool LockToCPU(int) {
  return false;
}

#else

#include <errno.h>
#include <sched.h>

bool LockToCPU(int lock_cpu) {
  cpu_set_t cpuset;

  CPU_ZERO(&cpuset);
  CPU_SET(lock_cpu, &cpuset);
  if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
    if (errno == EINVAL) {
      printf("Invalid cpu %d\n", lock_cpu);
    } else {
      perror("sched_setaffinity failed");
    }
    return false;
  }

  printf("Locked to cpu %d\n", lock_cpu);
  return true;
}

#endif

int main(int argc, char** argv) {
#if defined(__BIONIC__)
  // Enable decay time option to allow frees to run faster at the cost of slightly increasing RSS.
  // All applications on Android run with this option enabled.
  mallopt(M_DECAY_TIME, 1);
#endif
  std::vector<char*> new_argv;
  // The first argument is not an option, so add it as is.
  new_argv.push_back(argv[0]);

  // Look for the special option --benchmark_lock_cpu.
  int lock_cpu = -1;
  for (int i = 1; i < argc; i++) {
    if (android::base::StartsWith(argv[i], "--benchmark_cpu=")) {
      char* endptr;
      long cpu = strtol(&argv[i][16], &endptr, 10);
      if (endptr == nullptr || *endptr != '\0' || cpu > INT_MAX || cpu < 0) {
        printf("Malformed value for --benchmark_cpu, requires a valid positive number.\n");
        return 1;
      }
      lock_cpu = cpu;
    } else {
      new_argv.push_back(argv[i]);
    }
  }
  new_argv.push_back(nullptr);

  if (lock_cpu != -1 && !LockToCPU(lock_cpu)) {
    return 1;
  }

  int new_argc = new_argv.size() - 1;
  ::benchmark::Initialize(&new_argc, new_argv.data());
  if (::benchmark::ReportUnrecognizedArguments(new_argc, new_argv.data())) return 1;
  ::benchmark::RunSpecifiedBenchmarks();
}
