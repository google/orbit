//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitProcess.h"

#include "Core.h"
#include "Injection.h"
#include "OrbitModule.h"
#include "OrbitSession.h"
#include "OrbitThread.h"
#include "OrbitType.h"
#include "Path.h"
#include "Pdb.h"
#include "ScopeTimer.h"
#include "Serialization.h"

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
Process::Process(DWORD a_ID) {
  m_ID = a_ID;
  m_Handle = 0;
  m_Is64Bit = false;
  m_CpuUsage = 0;
  m_DebugInfoLoaded = false;
  m_IsRemote = false;
  m_IsElevated = false;
  Init();
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
void Process::Init() {
#ifdef _WIN32
  m_Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_ID);
  m_Is64Bit = ProcessUtils::Is64Bit(m_Handle);
  m_IsElevated = IsElevated(m_Handle);
  m_UpdateCpuTimer.Start();
#else
  m_Is64Bit = LinuxUtils::Is64Bit(m_ID);
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
    /*string symbolPath = Path::GetDirectory(this->GetFullName()).c_str();
    SymSetSearchPath(m_Handle, symbolPath.c_str());*/

    // List threads
    // EnumerateThreads();
    m_DebugInfoLoaded = true;
  }
#endif
}

//-----------------------------------------------------------------------------
void Process::SetID(DWORD a_ID) {
  m_ID = a_ID;
  Init();
}

//-----------------------------------------------------------------------------
void Process::ListModules() {
  SCOPE_TIMER_LOG("ListModules");

  ClearTransients();

#ifdef _WIN32
  SymUtils::ListModules(m_Handle, m_Modules);
#else
  LinuxUtils::ListModules(m_ID, m_Modules);
#endif

  for (auto& pair : m_Modules) {
    std::shared_ptr<Module>& module = pair.second;
    std::string name = ToLower(module->m_Name);
    m_NameToModuleMap[name] = module;
#ifdef _WIN32
    // TODO: check if the windows implementation does something meaningfull
    module->LoadDebugInfo();
#endif
  }
}

//-----------------------------------------------------------------------------
void Process::ClearTransients() {
  m_Functions.clear();
  m_Types.clear();
  m_Globals.clear();
  m_WatchedVariables.clear();
  m_NameToModuleMap.clear();
}

//-----------------------------------------------------------------------------
void Process::EnumerateThreads() {
  m_Threads.clear();
  m_ThreadIds.clear();

#ifdef _WIN32
  // https://blogs.msdn.microsoft.com/oldnewthing/20060223-14/?p=32173/
  HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, m_ID);
  if (h != INVALID_HANDLE_VALUE) {
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(h, &te)) {
      do {
        if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                             sizeof(te.th32OwnerProcessID)) {
          HANDLE thandle;
          { thandle = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID); }

          if (thandle == NULL) {
            // ORBIT_LOG(GetLastErrorAsString());
            continue;
          }

          if (te.th32OwnerProcessID == m_ID) {
            std::shared_ptr<Thread> thread = std::make_shared<Thread>();
            thread->m_Handle = thandle;
            thread->m_TID = te.th32ThreadID;
            m_ThreadNames[thread->m_TID] = GetThreadName(thandle);
            m_Threads.push_back(thread);
          }
        }
        te.dwSize = sizeof(te);
      } while (Thread32Next(h, &te));
    }

    CloseHandle(h);
  }

  for (std::shared_ptr<Thread>& thread : m_Threads) {
    m_ThreadIds.insert(thread->m_TID);
  }
#endif
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
void Process::UpdateThreadUsage() {
  for (auto& thread : m_Threads) {
    thread->UpdateUsage();
  }
}

//-----------------------------------------------------------------------------
void Process::SortThreadsByUsage() {
  std::sort(m_Threads.begin(), m_Threads.end(),
            [](std::shared_ptr<Thread>& a_T0, std::shared_ptr<Thread>& a_T1) {
              return a_T1->m_Usage.Latest() < a_T0->m_Usage.Latest();
            });
}

//-----------------------------------------------------------------------------
void Process::SortThreadsById() {
  std::sort(m_Threads.begin(), m_Threads.end(),
            [](std::shared_ptr<Thread>& a_T1, std::shared_ptr<Thread>& a_T0) {
              return a_T1->m_TID < a_T0->m_TID;
            });
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
std::shared_ptr<Module> Process::GetModuleFromAddress(DWORD64 a_Address) {
  DWORD64 address = (DWORD64)a_Address;
  auto it = m_Modules.upper_bound(address);
  if (!m_Modules.empty() && it != m_Modules.begin()) {
    --it;
    std::shared_ptr<Module> module = it->second;
    return module;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Module> Process::GetModuleFromName(const std::string& a_Name) {
  auto iter = m_NameToModuleMap.find(a_Name);
  if (iter != m_NameToModuleMap.end()) {
    return iter->second;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
void Process::AddSymbol(uint64_t a_Address,
                        std::shared_ptr<LinuxSymbol> a_Symbol) {
  m_Symbols[a_Address] = a_Symbol;
}

#ifdef _WIN32
//-----------------------------------------------------------------------------
std::shared_ptr<OrbitDiaSymbol> Process::SymbolFromAddress(DWORD64 a_Address) {
  std::shared_ptr<Module> module = GetModuleFromAddress(a_Address);
  if (module && module->m_Pdb) {
    return module->m_Pdb->SymbolFromAddress(a_Address);
  }
  return std::make_shared<OrbitDiaSymbol>();
}
#endif

//-----------------------------------------------------------------------------
bool Process::LineInfoFromAddress(DWORD64 a_Address, LineInfo& o_LineInfo) {
#ifdef _WIN32
  std::shared_ptr<Module> module = GetModuleFromAddress(a_Address);
  if (module && module->m_Pdb) {
    return module->m_Pdb->LineInfoFromAddress(a_Address, o_LineInfo);
  }
#endif

  return false;
}

//-----------------------------------------------------------------------------
void Process::LoadSession(const Session& a_Session) {}

//-----------------------------------------------------------------------------
void Process::SaveSession() {}

//-----------------------------------------------------------------------------
void Process::RefreshWatchedVariables() {
  for (std::shared_ptr<Variable> var : m_WatchedVariables) {
    var->SyncValue();
  }
}

//-----------------------------------------------------------------------------
void Process::ClearWatchedVariables() { m_WatchedVariables.clear(); }

//-----------------------------------------------------------------------------
void Process::AddType(Type& a_Type) {
  bool isPtr = a_Type.m_Name.find("Pointer to") != std::string::npos;
  if (!isPtr) {
    unsigned long long typeHash = a_Type.Hash();
    auto it = m_UniqueTypeHash.insert(typeHash);
    if (it.second == true) {
      m_Types.push_back(&a_Type);
    }
  }
}

//-----------------------------------------------------------------------------
void Process::AddModule(std::shared_ptr<Module>& a_Module) {
  m_Modules[a_Module->m_AddressStart] = a_Module;
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

    if (!module->m_FoundPdb) {
      std::string moduleName = ToLower(module->m_Name);
      std::string pdbName = Path::StripExtension(moduleName) + ".pdb";

      const std::vector<std::string>& pdbs = nameToPaths[pdbName];

      for (const std::string& pdb : pdbs) {
        module->m_PdbName = pdb;
        module->m_FoundPdb = true;
        module->LoadDebugInfo();

        std::string signature = GuidToString(module->m_Pdb->GetGuid());

        if (Contains(module->m_DebugSignature, signature)) {
          // Found matching pdb
          module->m_PdbSize = Path::FileSize(module->m_PdbName);
          break;
        } else {
          module->m_FoundPdb = false;
        }
      }
    }
  }
#endif
}

//-----------------------------------------------------------------------------
void Process::FillModuleDebugInfo(ModuleDebugInfo& a_ModuleDebugInfo) {
  PRINT_VAR(m_FullName);

  // Get module from name
  std::string name = ToLower(a_ModuleDebugInfo.m_Name);
  std::shared_ptr<Module> module = GetModuleFromName(name);

  if (module) {
    PRINT_VAR(module->m_FullName);
    module->LoadDebugInfo();
    const std::string& moduleName = module->m_FullName;
    module->m_Pdb->LoadPdb(moduleName.c_str());
    a_ModuleDebugInfo.m_Functions = module->m_Pdb->GetFunctions();
    a_ModuleDebugInfo.load_bias = module->m_Pdb->GetLoadBias();
  } else {
    PRINT_VAR(a_ModuleDebugInfo.m_Name);
    for (auto& pair : m_NameToModuleMap) {
      PRINT_VAR(pair.first);
    }
  }
}

//-----------------------------------------------------------------------------
bool Process::IsElevated(HANDLE a_Process) {
  bool fRet = false;
#ifdef _WIN32
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

#endif
  return fRet;
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
    PRINT("LookupPrivilegeValue error: ");
    PRINT_VAR(GetLastErrorAsString());
    return false;
  }

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = a_Enable ? SE_PRIVILEGE_ENABLED : 0;

  if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                             (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
    ORBIT_ERROR;
    PRINT("AdjustTokenPrivileges error: ");
    PRINT_VAR(GetLastErrorAsString());
    return false;
  }

  if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
    PRINT("The token does not have the specified privilege. \n");
    return false;
  }

  return true;
}
#endif

//-----------------------------------------------------------------------------
DWORD64 Process::GetOutputDebugStringAddress() {
#ifdef _WIN32
  auto it = m_NameToModuleMap.find("kernelbase.dll");
  if (it != m_NameToModuleMap.end()) {
    std::shared_ptr<Module> module = it->second;
    auto remoteAddr = Injection::GetRemoteProcAddress(
        GetHandle(), module->m_ModuleHandle, "OutputDebugStringA");
    return (DWORD64)remoteAddr;
  }

#endif
  return 0;
}

//-----------------------------------------------------------------------------
DWORD64 Process::GetRaiseExceptionAddress() {
#ifdef _WIN32
  auto it = m_NameToModuleMap.find("kernelbase.dll");
  if (it != m_NameToModuleMap.end()) {
    std::shared_ptr<Module> module = it->second;
    auto remoteAddr = Injection::GetRemoteProcAddress(
        GetHandle(), module->m_ModuleHandle, "RaiseException");
    return (DWORD64)remoteAddr;
  }
#endif

  return 0;
}

//-----------------------------------------------------------------------------
void Process::FindCoreFunctions() {
#if 0
    SCOPE_TIMER_LOG("FindCoreFunctions");

    const auto prio = oqpi::task_priority::normal;

    oqpi_tk::parallel_for( "FindAllocFreeFunctions", (int32_t)m_Functions.size(), [&]( int32_t a_BlockIndex, int32_t a_ElementIndex )
    {
        Function* func = m_Functions[a_ElementIndex];
        const std::string & name = func->Lower();

        if( Contains( name, L"operator new" ) || Contains( name, L"FMallocBinned::Malloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::ALLOC;
        }
        else if( Contains( name, L"operator delete" ) || name == L"FMallocBinned::Free" )
        {
            func->Select();
            func->m_OrbitType = Function::FREE;
        } 
        else if( Contains( name, L"realloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::REALLOC;
        }
    } );
#endif
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(Process, 1) {
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_FullName);
  ORBIT_NVP_VAL(0, m_ID);
  ORBIT_NVP_VAL(0, m_IsElevated);
  ORBIT_NVP_VAL(0, m_CpuUsage);
  ORBIT_NVP_VAL(0, m_Is64Bit);
  ORBIT_NVP_VAL(0, m_DebugInfoLoaded);
  ORBIT_NVP_VAL(0, m_IsRemote);
  ORBIT_NVP_VAL(0, m_Modules);
  ORBIT_NVP_VAL(0, m_NameToModuleMap);
  ORBIT_NVP_VAL(0, m_ThreadIds);
  ORBIT_NVP_VAL(1, m_Modules);
}
