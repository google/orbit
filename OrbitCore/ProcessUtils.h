//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

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
  void SortByID();
  void SortByName();
  void SortByCPU();
  void UpdateCpuTimes();
  void SetRemote(bool value);
  bool Contains(uint32_t pid) const;

  std::shared_ptr<Process> GetProcess(uint32_t pid) const;
  const std::vector<std::shared_ptr<Process>>& GetProcesses() const;

  size_t Size() const;
  void AddProcess(std::shared_ptr<Process> process);

  ORBIT_SERIALIZABLE;
 private:
  std::vector<std::shared_ptr<Process>> processes_;
  std::unordered_map<uint32_t, std::shared_ptr<Process>> processes_map_;

};

#endif  // ORBIT_CORE_PROCESS_UTILS_H_
