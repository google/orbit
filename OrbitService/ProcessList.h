// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_PROCESS_LIST_
#define ORBIT_SERVICE_PROCESS_LIST_

#include <outcome.hpp>
#include <vector>

#include "process.pb.h"

class ProcessList {
 public:
  outcome::result<void, std::string> Refresh();
  const std::vector<ProcessInfo>& GetProcesses() { return processes_; }

 private:
  std::vector<ProcessInfo> processes_;
  std::unordered_map<int32_t, ProcessInfo*> processes_map_;
};

#endif  // ORBIT_SERVICE_PROCESS_LIST_