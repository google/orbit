//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ProcessUtils.h"

#include <memory>

#include "OrbitBase/Logging.h"

#ifdef _WIN32
#include <tlhelp32.h>
#else
#include <dirent.h>
#include <sys/stat.h>   // for stat()
#include <sys/types.h>  // for opendir(), readdir(), closedir()
#include <unistd.h>

#define PROC_DIRECTORY "/proc/"
#define CASE_SENSITIVE 1
#define CASE_INSENSITIVE 0
#define EXACT_MATCH 1
#define INEXACT_MATCH 0
#endif

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <streambuf>

#include "absl/strings/str_format.h"
#include "absl/strings/strip.h"

// Is64BitProcess function taken from Very Sleepy
#ifdef _WIN64
typedef BOOL WINAPI Wow64GetThreadContext_t(__in HANDLE hThread,
                                            __inout PWOW64_CONTEXT lpContext);
typedef DWORD WINAPI Wow64SuspendThread_t(__in HANDLE hThread);
Wow64GetThreadContext_t* fn_Wow64GetThreadContext =
    (Wow64GetThreadContext_t*)GetProcAddress(GetModuleHandle(L"kernel32"),
                                             "Wow64GetThreadContext");
Wow64SuspendThread_t* fn_Wow64SuspendThread =
    (Wow64SuspendThread_t*)GetProcAddress(GetModuleHandle(L"kernel32"),
                                          "Wow64SuspendThread");
#endif

bool ProcessUtils::Is64Bit(HANDLE hProcess) {
#ifdef _WIN32
  // https://github.com/VerySleepy/verysleepy/blob/master/src/utils/osutils.cpp

  typedef BOOL WINAPI IsWow64Process_t(__in HANDLE hProcess,
                                       __out PBOOL Wow64Process);
  static bool first = true;
  static IsWow64Process_t* IsWow64ProcessPtr = NULL;

#ifndef _WIN64
  static BOOL isOn64BitOs = 0;
#endif

  if (first) {
    first = false;
    IsWow64ProcessPtr = (IsWow64Process_t*)GetProcAddress(
        GetModuleHandle(L"kernel32"), "IsWow64Process");
#ifndef _WIN64
    if (!IsWow64ProcessPtr) return false;
    IsWow64ProcessPtr(GetCurrentProcess(), &isOn64BitOs);
#endif
  }

#ifndef _WIN64
  if (!isOn64BitOs) {
    return false;
  }
#endif

  if (IsWow64ProcessPtr) {
    BOOL isWow64Process;
    if (IsWow64ProcessPtr(hProcess, &isWow64Process) && !isWow64Process) {
      return true;
    }
  }
#else
  UNUSED(hProcess);
#endif
  return false;
}

void ProcessList::Clear() {
  processes_.clear();
  processes_map_.clear();
}

void ProcessList::Refresh() {
#ifdef _WIN32
  processes_.clear();
  std::unordered_map<uint32_t, std::shared_ptr<Process>> previousProcessesMap =
      processes_map_;
  processes_map_.clear();

  HANDLE snapshot = CreateToolhelp32Snapshot(
      TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE /*| TH32CS_SNAPTHREAD*/, 0);
  PROCESSENTRY32 processinfo;
  processinfo.dwSize = sizeof(PROCESSENTRY32);

  if (Process32First(snapshot, &processinfo)) {
    do {
      if (processinfo.th32ProcessID == GetCurrentProcessId()) continue;

      //#ifndef _WIN64
      //            // If the process is 64 bit, skip it.
      //            if (Is64BitProcess(process_handle)) {
      //                CloseHandle(process_handle);
      //                continue;
      //            }
      //#else
      //            // Skip 32 bit processes on system that does not have the
      //            needed functions (Windows XP 64). if (/*TODO:*/
      //            (fn_Wow64SuspendThread == NULL || fn_Wow64GetThreadContext
      //            == NULL) && !ProcessUtils::Is64Bit(process_handle)) {
      //                CloseHandle(process_handle);
      //                continue;
      //            }
      //#endif

      auto it = previousProcessesMap.find(processinfo.th32ProcessID);
      if (it != previousProcessesMap.end()) {
        // Add existing process
        processes_.push_back(it->second);
      } else {
        // Process was not there previously
        auto process = std::make_shared<Process>();
        process->SetName(ws2s(processinfo.szExeFile));
        process->SetID(processinfo.th32ProcessID);

        // Full path
        HANDLE moduleSnapshot = CreateToolhelp32Snapshot(
            TH32CS_SNAPMODULE, processinfo.th32ProcessID);
        if (moduleSnapshot != INVALID_HANDLE_VALUE) {
          MODULEENTRY32 moduleEntry;
          moduleEntry.dwSize = sizeof(MODULEENTRY32);
          BOOL res = Module32First(moduleSnapshot, &moduleEntry);
          if (res) {
            process->SetFullPath(ws2s(moduleEntry.szExePath));
            // TODO: Append arguments.
            process->SetCmdLine(process->GetFullPath());
          } else {
            ERROR("Call to Module32First failed for %s (pid=%d)",
                  process->GetName(), processinfo.th32ProcessID);
          }

          CloseHandle(moduleSnapshot);
        }

        processes_.push_back(process);
      }

      processes_map_[processinfo.th32ProcessID] = processes_.back();

    } while (Process32Next(snapshot, &processinfo));
  }
  CloseHandle(snapshot);

#else
  processes_.clear();
  struct dirent* de_DirEntity = nullptr;
  DIR* dir_proc = nullptr;

  dir_proc = opendir(PROC_DIRECTORY);
  if (dir_proc == nullptr) {
    perror("Couldn't open the " PROC_DIRECTORY " directory");
    return;
  }

  while ((de_DirEntity = readdir(dir_proc))) {
    if (de_DirEntity->d_type == DT_DIR && IsAllDigits(de_DirEntity->d_name)) {
      int pid = atoi(de_DirEntity->d_name);
      auto iter = processes_map_.find(pid);
      std::shared_ptr<Process> process = nullptr;
      if (iter == processes_map_.end()) {
        process = std::make_shared<Process>();
        std::string dir =
            absl::StrFormat("%s%s/", PROC_DIRECTORY, de_DirEntity->d_name);
        std::string name = FileToString(dir + "comm");
        // Remove new line character.
        absl::StripTrailingAsciiWhitespace(&name);
        process->SetName(name);

        // "The command-line arguments appear [...] as a set of strings
        // separated by null bytes ('\0')".
        std::string cmdline = FileToString(dir + "cmdline");
        process->SetFullPath(cmdline.substr(0, cmdline.find('\0')));

        std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');
        process->SetCmdLine(cmdline);

        process->SetID(pid);
        processes_map_[pid] = process;
      } else {
        process = iter->second;
      }

      processes_.push_back(process);
    }
  }
  closedir(dir_proc);
#endif
}

void ProcessList::SortByID() {
  std::sort(processes_.begin(), processes_.end(),
            [](std::shared_ptr<Process>& a_P1, std::shared_ptr<Process>& a_P2) {
              return a_P1->GetID() < a_P2->GetID();
            });
}

void ProcessList::SortByName() {
  std::sort(processes_.begin(), processes_.end(),
            [](std::shared_ptr<Process>& a_P1, std::shared_ptr<Process>& a_P2) {
              return a_P1->GetName() < a_P2->GetName();
            });
}

void ProcessList::SortByCPU() {
  std::sort(processes_.begin(), processes_.end(),
            [](std::shared_ptr<Process>& a_P1, std::shared_ptr<Process>& a_P2) {
              return a_P1->GetCpuUsage() < a_P2->GetCpuUsage();
            });
}

void ProcessList::UpdateCpuTimes() {
#ifdef WIN32
  for (std::shared_ptr<Process>& process : processes_) {
    process->UpdateCpuTime();
  }
#else
  std::unordered_map<uint32_t, float> processMap =
      LinuxUtils::GetCpuUtilization();
  for (std::shared_ptr<Process>& process : processes_) {
    uint32_t pid = process->GetID();
    process->SetCpuUsage(processMap[pid]);
  }
#endif
}

bool ProcessList::Contains(uint32_t pid) const {
  return processes_map_.find(pid) != processes_map_.end();
}

void ProcessList::SetRemote(bool value) {
  for (std::shared_ptr<Process>& process : processes_) {
    process->SetIsRemote(value);
  }
}

std::shared_ptr<Process> ProcessList::GetProcess(uint32_t pid) const {
  auto iter = processes_map_.find(pid);
  if (iter != processes_map_.end()) {
    return iter->second;
  }

  return nullptr;
}

const std::vector<std::shared_ptr<Process>>& ProcessList::GetProcesses() const {
  return processes_;
}

size_t ProcessList::Size() const { return processes_.size(); }

void ProcessList::AddProcess(std::shared_ptr<Process> process) {
  uint32_t pid = process->GetID();
  auto it = processes_map_.find(pid);

  if (it != processes_map_.end()) {
    // Process with this pid already in the list - do nothing..
    ERROR("ProcessList already contains process with pid=%d - ignoring", pid);
    return;
  }

  processes_.push_back(process);
  processes_map_[pid] = process;
}

ORBIT_SERIALIZE(ProcessList, 0) {
  ORBIT_NVP_VAL(0, processes_);
  ORBIT_NVP_VAL(0, processes_map_);
}
