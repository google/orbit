// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitProcess.h"

#include <OrbitBase/Logging.h>
#include <absl/strings/ascii.h>

#include <utility>

#include "Core.h"
#include "Injection.h"
#include "OrbitModule.h"
#include "OrbitSession.h"
#include "OrbitThread.h"
#include "Path.h"
#include "Pdb.h"
#include "ScopeTimer.h"
#include "Utils.h"

#ifdef _WIN32
#include <tlhelp32.h>

#include "SymbolUtils.h"
#else
#include "LinuxUtils.h"
#endif

//-----------------------------------------------------------------------------
Process::Process() {
  m_ID = 0;
  m_Handle = 0;
  m_Is64Bit = false;
  m_CpuUsage = 0;
  m_DebugInfoLoaded = false;
  m_IsRemote = false;
  m_IsElevated = false;
}

//-----------------------------------------------------------------------------
Process::~Process() {
#ifdef _WIN32
  if (m_DebugInfoLoaded) {
    OrbitSymCleanup(m_Handle);
  }
#endif
}

//-----------------------------------------------------------------------------
void Process::LoadDebugInfo() {
#ifdef _WIN32
  if (!m_DebugInfoLoaded) {
    if (m_Handle == nullptr) {
      m_Handle = GetCurrentProcess();
    }

    // Initialize dbghelp
    // SymInit(m_Handle);

    // Load module information
    /*string symbolPath = Path::GetDirectory(this->GetFullPath()).c_str();
    SymSetSearchPath(m_Handle, symbolPath.c_str());*/

    // List threads
    // EnumerateThreads();
    m_DebugInfoLoaded = true;
  }
#endif
}

//-----------------------------------------------------------------------------
void Process::SetID(int32_t id) { m_ID = id; }

//-----------------------------------------------------------------------------
void Process::ClearTransients() {
  m_Functions.clear();
  m_NameToModuleMap.clear();
  path_to_module_map_.clear();
}

//-----------------------------------------------------------------------------
void Process::UpdateCpuTime() {
#ifdef _WIN32
  FILETIME creationTime;
  FILETIME exitTime;
  FILETIME kernTime;
  FILETIME userTime;

  double elapsedMillis = m_UpdateCpuTimer.QueryMillis();
  m_UpdateCpuTimer.Start();

  if (GetProcessTimes(m_Handle, &creationTime, &exitTime, &kernTime,
                      &userTime)) {
    unsigned numCores = std::thread::hardware_concurrency();
    LONGLONG kernMs = FileTimeDiffInMillis(m_LastKernTime, kernTime);
    LONGLONG userMs = FileTimeDiffInMillis(m_LastUserTime, userTime);
    m_LastKernTime = kernTime;
    m_LastUserTime = userTime;
    m_CpuUsage = (100.0 * double(kernMs + userMs) / elapsedMillis) / numCores;
  }
#endif
}

//-----------------------------------------------------------------------------
std::shared_ptr<Module> Process::FindModule(const std::string& module_name) {
  std::string moduleName = ToLower(Path::GetFileNameNoExt(module_name));

  for (auto& it : m_Modules) {
    std::shared_ptr<Module>& module = it.second;
    if (ToLower(Path::GetFileNameNoExt(module->m_Name)) == moduleName) {
      return module;
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
Function* Process::GetFunctionFromAddress(uint64_t address, bool a_IsExact) {
  if (m_Modules.empty()) {
    return nullptr;
  }

  auto it = m_Modules.upper_bound(address);
  if (it == m_Modules.begin()) {
    return nullptr;
  }

  --it;
  std::shared_ptr<Module>& module = it->second;
  if (address >= module->m_AddressEnd) {
    return nullptr;
  }

  if (module->m_Pdb == nullptr) {
    return nullptr;
  }

  if (a_IsExact) {
    return module->m_Pdb->GetFunctionFromExactAddress(address);
  } else {
    return module->m_Pdb->GetFunctionFromProgramCounter(address);
  }
}

//-----------------------------------------------------------------------------
std::shared_ptr<Module> Process::GetModuleFromAddress(uint64_t a_Address) {
  if (m_Modules.empty()) {
    return nullptr;
  }

  auto module_it = m_Modules.upper_bound(a_Address);
  if (module_it == m_Modules.begin()) {
    return nullptr;
  }

  --module_it;
  std::shared_ptr<Module> module = module_it->second;
  CHECK(a_Address >= module->m_AddressStart);
  if (a_Address >= module->m_AddressEnd) {
    return nullptr;
  }

  return module;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Module> Process::GetModuleFromName(const std::string& a_Name) {
  auto iter = m_NameToModuleMap.find(absl::AsciiStrToLower(a_Name));
  if (iter != m_NameToModuleMap.end()) {
    return iter->second;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Module> Process::GetModuleFromPath(
    const std::string& module_path) {
  auto iter = path_to_module_map_.find(module_path);
  if (iter != path_to_module_map_.end()) {
    return iter->second;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
bool Process::LineInfoFromAddress(uint64_t /*a_Address*/,
                                  LineInfo& /*o_LineInfo*/) {
  // TODO(b/158093728): Dia Loading was disabled, reimplement using LLVM.
  return false;
}

//-----------------------------------------------------------------------------
void Process::LoadSession(const Session&) {}

//-----------------------------------------------------------------------------
void Process::SavePreset() {}

//-----------------------------------------------------------------------------
void Process::AddModule(std::shared_ptr<Module>& a_Module) {
  m_Modules[a_Module->m_AddressStart] = a_Module;
  m_NameToModuleMap[absl::AsciiStrToLower(a_Module->m_Name)] = a_Module;
  path_to_module_map_[a_Module->m_FullName] = a_Module;
}

//-----------------------------------------------------------------------------
void Process::FindPdbs(const std::vector<std::string>& a_SearchLocations) {
#ifdef _WIN32
  std::unordered_map<std::string, std::vector<std::string> > nameToPaths;

  // Populate list of all available pdb files
  for (const std::string& dir : a_SearchLocations) {
    std::vector<std::string> pdbFiles = Path::ListFiles(dir, ".pdb");
    for (const std::string& pdb : pdbFiles) {
      std::string pdbLower = Path::GetFileName(ToLower(pdb));
      nameToPaths[pdbLower].push_back(pdb);
    }
  }

  // Find matching pdb
  for (auto& modulePair : m_Modules) {
    std::shared_ptr<Module> module = modulePair.second;

    if (!module->IsLoadable()) {
      std::string moduleName = ToLower(module->m_Name);
      std::string pdbName = Path::StripExtension(moduleName) + ".pdb";

      const std::vector<std::string>& pdbs = nameToPaths[pdbName];

      for (const std::string& pdb : pdbs) {
        module->m_PdbName = pdb;
        module->SetLoadable(true);
        module->LoadDebugInfo();

        std::string signature = GuidToString(module->m_Pdb->GetGuid());

        if (absl::StrContains(module->m_DebugSignature, signature)) {
          // Found matching pdb
          module->m_PdbSize = Path::FileSize(module->m_PdbName);
          break;
        } else {
          module->SetLoadable(false);
        }
      }
    }
  }
#else
  UNUSED(a_SearchLocations);
#endif
}

//-----------------------------------------------------------------------------
bool Process::IsElevated(HANDLE a_Process) {
#ifdef _WIN32
  bool fRet = false;
  HANDLE hToken = NULL;
  if (OpenProcessToken(a_Process, TOKEN_QUERY, &hToken)) {
    TOKEN_ELEVATION Elevation;
    DWORD cbSize = sizeof(TOKEN_ELEVATION);
    if (GetTokenInformation(hToken, TokenElevation, &Elevation,
                            sizeof(Elevation), &cbSize)) {
      fRet = Elevation.TokenIsElevated != 0;
    }
  }
  if (hToken) {
    CloseHandle(hToken);
  }
  return fRet;
#else
  UNUSED(a_Process);
  return false;
#endif
}

#ifdef _WIN32
//-----------------------------------------------------------------------------
bool Process::SetPrivilege(LPCTSTR a_Name, bool a_Enable) {
  HANDLE hToken;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
                        &hToken)) {
    ORBIT_ERROR;
    PRINT_VAR(GetLastErrorAsString());
  }

  TOKEN_PRIVILEGES tp;
  LUID luid;
  if (!LookupPrivilegeValue(NULL, a_Name, &luid)) {
    ORBIT_ERROR;
    LOG("LookupPrivilegeValue error: ");
    PRINT_VAR(GetLastErrorAsString());
    return false;
  }

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = a_Enable ? SE_PRIVILEGE_ENABLED : 0;

  if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                             (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
    ORBIT_ERROR;
    LOG("AdjustTokenPrivileges error: ");
    PRINT_VAR(GetLastErrorAsString());
    return false;
  }

  if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
    LOG("The token does not have the specified privilege.");
    return false;
  }

  return true;
}
#endif

//-----------------------------------------------------------------------------
uint64_t Process::GetOutputDebugStringAddress() {
#ifdef _WIN32
  auto it = m_NameToModuleMap.find("kernelbase.dll");
  if (it != m_NameToModuleMap.end()) {
    std::shared_ptr<Module> module = it->second;
    auto remoteAddr = Injection::GetRemoteProcAddress(
        GetHandle(), module->m_ModuleHandle, "OutputDebugStringA");
    return reinterpret_cast<uintptr_t>(remoteAddr);
  }

#endif
  return 0;
}

//-----------------------------------------------------------------------------
uint64_t Process::GetRaiseExceptionAddress() {
#ifdef _WIN32
  auto it = m_NameToModuleMap.find("kernelbase.dll");
  if (it != m_NameToModuleMap.end()) {
    std::shared_ptr<Module> module = it->second;
    auto remoteAddr = Injection::GetRemoteProcAddress(
        GetHandle(), module->m_ModuleHandle, "RaiseException");
    return reinterpret_cast<uintptr_t>(remoteAddr);
  }
#endif

  return 0;
}
