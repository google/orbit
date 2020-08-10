// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifdef _WIN32
#include <BaseTsd.h>
#include <tchar.h>
#include <wtypes.h>
#endif

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>

inline pid_t GetCurrentThreadId() {
  thread_local pid_t current_tid = syscall(__NR_gettid);
  return current_tid;
}
#endif
