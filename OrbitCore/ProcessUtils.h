// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#ifndef ORBIT_CORE_PROCESS_UTILS_H_
#define ORBIT_CORE_PROCESS_UTILS_H_

#include <unordered_map>

#include "OrbitProcess.h"
#include "Serialization.h"

namespace ProcessUtils {
bool Is64Bit(HANDLE hProcess);
}

class ProcessList {
 public:
  void Refresh();
  void Clear();
  void UpdateCpuTimes();

  bool Contains(int32_t pid) const;
  std::shared_ptr<Process> GetProcess(int32_t pid) const;
  const std::vector<std::shared_ptr<Process>>& GetProcesses() const;
  size_t Size() const;

  ORBIT_SERIALIZABLE;

 private:
  std::vector<std::shared_ptr<Process>> processes_;
  std::unordered_map<int32_t, std::shared_ptr<Process>> processes_map_;
};

#endif  // ORBIT_CORE_PROCESS_UTILS_H_
