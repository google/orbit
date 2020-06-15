// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "SymbolUtils.h"

#include <psapi.h>
#include <tlhelp32.h>

#include "Capture.h"
#include "OrbitDbgHelp.h"
#include "OrbitModule.h"

//-----------------------------------------------------------------------------
void SymUtils::ListModules(
    HANDLE a_ProcessHandle,
    std::map<DWORD64, std::shared_ptr<Module> >& o_ModuleMap) {
  SCOPE_TIMER_LOG("SymUtils::ListModules");

  const DWORD ModuleArraySize = 1024;
  DWORD NumModules = 0;
  TCHAR ModuleNameBuffer[MAX_PATH] = {0};
  TCHAR ModuleFullNameBuffer[MAX_PATH] = {0};
  HMODULE ModuleArray[1024];
  o_ModuleMap.clear();

  /* Get handles to all the modules in the target process */
  SetLastError(NO_ERROR);
  if (!::EnumProcessModulesEx(a_ProcessHandle, &ModuleArray[0],
                              ModuleArraySize * sizeof(HMODULE), &NumModules,
                              LIST_MODULES_ALL)) {
    std::string EnumProcessModulesExError = GetLastErrorAsString();
    PRINT_VAR(EnumProcessModulesExError);
    return;
  }

  NumModules /= sizeof(HMODULE);
  if (NumModules > ModuleArraySize) {
    PRINT_VAR("NumModules > ModuleArraySize");
    return;
  }

  for (DWORD i = 0; i <= NumModules; ++i) {
    HMODULE hModule = ModuleArray[i];
    GetModuleBaseName(a_ProcessHandle, hModule, ModuleNameBuffer,
                      sizeof(ModuleNameBuffer));
    GetModuleFileNameEx(a_ProcessHandle, hModule, ModuleFullNameBuffer,
                        sizeof(ModuleFullNameBuffer));

    MODULEINFO moduleInfo;
    memset(&moduleInfo, 0, sizeof(moduleInfo));
    GetModuleInformation(a_ProcessHandle, hModule, &moduleInfo,
                         sizeof(MODULEINFO));

    std::shared_ptr<Module> module = std::make_shared<Module>();
    module->m_Name = ws2s(ModuleNameBuffer);
    module->m_FullName = ws2s(ModuleFullNameBuffer);
    module->m_Directory = Path::GetDirectory(module->m_FullName);
    module->m_AddressStart = (DWORD64)moduleInfo.lpBaseOfDll;
    module->m_AddressEnd =
        (DWORD64)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;
    module->m_EntryPoint = (DWORD64)moduleInfo.EntryPoint;

    std::string filePath = module->m_FullName;
    Replace(filePath, ".exe", ".pdb");
    Replace(filePath, ".dll", ".pdb");
    if (Path::FileExists(filePath)) {
      module->SetLoadable(true);
      module->m_PdbSize = Path::FileSize(filePath);
      module->m_PdbName = filePath;
    }

    module->m_ModuleHandle = hModule;

    if (module->m_AddressStart != 0) {
      o_ModuleMap[module->m_AddressStart] = module;
    }
  }
}

//-----------------------------------------------------------------------------
bool SymUtils::GetLineInfo(DWORD64 a_Address, LineInfo& o_LineInfo) {
  std::shared_ptr<Process> process = Capture::GTargetProcess;

  if (process) {
    return process->LineInfoFromAddress(a_Address, o_LineInfo);
  }

  return false;
}

//-----------------------------------------------------------------------------
ScopeSymCleanup::ScopeSymCleanup(HANDLE a_Handle) : m_Handle(a_Handle) {}

//-----------------------------------------------------------------------------
ScopeSymCleanup::~ScopeSymCleanup() {
  if (!SymCleanup(m_Handle)) {
    ORBIT_ERROR;
  }
}
