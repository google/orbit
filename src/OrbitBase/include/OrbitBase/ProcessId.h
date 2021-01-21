// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_LINUX_PROC_H_
#define ORBIT_BASE_LINUX_PROC_H_

#include <sys/types.h>

#include <vector>

namespace orbit_base {

#if defined(__linux)
std::vector<pid_t> GetAllPids();

std::vector<pid_t> GetTidsOfProcess(pid_t pid);

std::vector<pid_t> GetAllTids();
#endif

}  // namespace orbit_base

#endif  // ORBIT_BASE_LINUX_PROC_H_