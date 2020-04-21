//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#include "Core.h"
#include "ProcessUtils.h"

class Injection {
 public:
  Injection();

  bool Inject(std::string_view a_RemoteAddress, const std::string& a_Dll,
              const Process& a_Process, const std::string& ProcName);
  DWORD GetProcessID() const { return m_InjectedProcessID; }
  HANDLE GetProcessHandle() const { return m_InjectedProcessHandle; }

  static HANDLE GetTargetProcessHandle(const std::string& a_Target,
                                       DWORD& o_ProcessID);

#ifdef _WIN32
  static HMODULE WINAPI GetRemoteModuleHandle(HANDLE hProcess,
                                              LPCSTR lpModuleName);
  static FARPROC WINAPI GetRemoteProcAddress(HANDLE hProcess, HMODULE hModule,
                                             LPCSTR lpProcName,
                                             UINT Ordinal = 0,
                                             BOOL UseOrdinal = FALSE);
#endif

 protected:
  void* RemoteWrite(const char* data, size_t size);
  void* RemoteWrite(const std::string& str);

 private:
  DWORD m_InjectedProcessID;
  HANDLE m_InjectedProcessHandle;
};
