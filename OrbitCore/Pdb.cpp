//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Pdb.h"

#include <ppl.h>

#include <algorithm>
#include <fstream>
#include <functional>

#include "Capture.h"
#include "Core.h"
#include "DiaManager.h"
#include "Log.h"
#include "ObjectCount.h"
#include "OrbitSession.h"
#include "OrbitType.h"
#include "OrbitUnreal.h"
#include "Params.h"
// clang-format off
#include "dia2dump.h"
// clang-format on
#include "PrintSymbol.h"
#include "Serialization.h"
#include "SymbolUtils.h"
#include "Tcp.h"
#include "TcpServer.h"
#include "Utils.h"
#include "absl/strings/str_format.h"

std::shared_ptr<Pdb> GPdbDbg;

//-----------------------------------------------------------------------------
Pdb::Pdb(const char* pdb_name)
    : m_FileName(pdb_name),
      m_MainModule(0),
      m_LastLoadTime(0),
      m_LoadedFromCache(false),
      m_FinishedLoading(false),
      m_IsLoading(false),
      m_IsPopulatingFunctionMap(false),
      m_IsPopulatingFunctionStringMap(false),
      m_DiaSession(nullptr),
      m_DiaGlobalSymbol(nullptr),
      m_DiaDataSource(nullptr) {
  m_Name = Path::GetFileName(m_FileName);
  memset(&m_ModuleInfo, 0, sizeof(IMAGEHLP_MODULE64));
  m_ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
  m_LoadTimer = new Timer();
}

//-----------------------------------------------------------------------------
Pdb::~Pdb() {
  if (m_DiaSession) {
    m_DiaSession->Release();
  }

  if (m_DiaGlobalSymbol) {
    m_DiaGlobalSymbol->Release();
  }

  if (m_DiaDataSource) {
    m_DiaDataSource->Release();
  }
}

//-----------------------------------------------------------------------------
void Pdb::AddFunction(Function& a_Function) {
  CheckOrbitFunction(a_Function);
  m_Functions.push_back(a_Function);
}

//-----------------------------------------------------------------------------
void Pdb::CheckOrbitFunction(Function& a_Function) {
  const std::string& name = a_Function.PrettyName();
  if (name == "OrbitStart") {
    a_Function.SetOrbitType(Function::ORBIT_TIMER_START);
  } else if (name == "OrbitStop") {
    a_Function.SetOrbitType(Function::ORBIT_TIMER_STOP);
  } else if (name == "OrbitLog") {
    a_Function.SetOrbitType(Function::ORBIT_LOG);
  } else if (name == "OutputDebugStringA") {
    a_Function.SetOrbitType(Function::ORBIT_OUTPUT_DEBUG_STRING);
  } else if (name == "OrbitSendData") {
    a_Function.SetOrbitType(Function::ORBIT_DATA);
  }
}

//-----------------------------------------------------------------------------
void Pdb::AddType(const Type& a_Type) {
  if (m_TypeMap.find(a_Type.m_Id) == m_TypeMap.end()) {
    m_TypeMap[a_Type.m_Id] = a_Type;
    m_Types.push_back(a_Type);
  }
}

//-----------------------------------------------------------------------------
void Pdb::AddGlobal(const Variable& a_Global) { m_Globals.push_back(a_Global); }

//-----------------------------------------------------------------------------
void Pdb::AddArgumentRegister(const std::string& a_Reg,
                              const std::string& a_Function) {
  m_ArgumentRegisters.insert(a_Reg);

  if (a_Reg.find("ESP") != std::string::npos) {
    m_RegFunctionsMap["ESP"].push_back(a_Function);
  } else if (a_Reg.find("30006") != std::string::npos) {
    m_RegFunctionsMap["30006"].push_back(a_Function);
  }
}

//-----------------------------------------------------------------------------
Type* Pdb::GetTypePtrFromId(ULONG a_ID) {
  auto it = m_TypeMap.find(a_ID);
  if (it != m_TypeMap.end()) {
    return &it->second;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
GUID Pdb::GetGuid() {
  GUID guid = {0};
  if (m_DiaGlobalSymbol) {
    m_DiaGlobalSymbol->get_guid(&guid);
  }

  return guid;
}

//-----------------------------------------------------------------------------
void Pdb::PrintFunction(Function& a_Function) { a_Function.Print(); }

//-----------------------------------------------------------------------------
void Pdb::Clear() {
  m_Functions.clear();
  m_Types.clear();
  m_Globals.clear();
  m_TypeMap.clear();
  m_FunctionMap.clear();
  m_FileName = "";
}

//-----------------------------------------------------------------------------
void Pdb::Reserve() {
  const int size = 8 * 1024;
  m_Functions.reserve(size);
  m_Types.reserve(size);
  m_Globals.reserve(size);
}

//-----------------------------------------------------------------------------
void Pdb::Print() const {
  for (auto& reg : m_ArgumentRegisters) {
    ORBIT_LOGV(reg);
  }

  ORBIT_LOG("\n\nEsp functions:");
  for (auto& reg : m_RegFunctionsMap) {
    ORBIT_LOGV(reg.first);
    for (auto& func : reg.second) {
      ORBIT_LOGV(func);
    }
  }
}

//-----------------------------------------------------------------------------
void Pdb::PrintGlobals() const {
  for (const auto& var : m_Globals) {
    PRINT_VAR(var.ToString());
  }
}

//-----------------------------------------------------------------------------
std::string Pdb::GetCachedName() {
  std::string pdbName = Path::GetFileName(m_FileName);
  std::string fileName = GuidToString(m_ModuleInfo.PdbSig70) + "-" +
                         ToHexString(m_ModuleInfo.PdbAge) + "_" + pdbName;
  fileName += ".bin";
  return fileName;
}

//-----------------------------------------------------------------------------
std::string Pdb::GetCachedKey() {
  std::string cachedName = GetCachedName();
  std::string cachedKey = cachedName.substr(0, cachedName.find_first_of('_'));
  return cachedKey;
}

//-----------------------------------------------------------------------------
bool Pdb::Load(const std::string& a_CachedPdb) {
  /*ifstream is( a_CachedPdb, std::ios::binary );
  if( !is.fail() )
  {
      cereal::BinaryInputArchive Ar( is );
      Ar(*this);
      m_LoadedFromCache = true;
      return true;
  }*/

  return false;
}

//-----------------------------------------------------------------------------
void Pdb::Update() {
  if (m_FinishedLoading) {
    m_LoadingCompleteCallback();
    m_FinishedLoading = false;
    Print();

    if (!m_LoadedFromCache && false) {
      Save();
    }
  }

  if (m_IsLoading) {
    SendStatusToUi();
  }
}

//-----------------------------------------------------------------------------
void Pdb::SendStatusToUi() {
  std::wstring status = Format(
      L"status:Parsing %s\nFunctions: %i\nTypes: %i\nGlobals: %i\n",
      m_Name.c_str(), m_Functions.size(), m_Types.size(), m_Globals.size());

  if (m_IsPopulatingFunctionMap) {
    status += L"PopulatingFunctionMap\n";
  }
  if (m_IsPopulatingFunctionStringMap) {
    status += L"PopulatingFunctionStringMap\n";
  }

  int numPoints = 10;
  int period = 4000;
  int progress = (int)(float(((ULONG)m_LoadTimer->QueryMillis() % period)) /
                       (float)period * (float)numPoints);
  if (progress > numPoints) progress = numPoints;
  for (int i = 0; i <= progress; ++i) {
    status += L".";
  }

  GTcpServer->SendToUiNow(status);
}

//-----------------------------------------------------------------------------
void ParseDll(const char* a_FileName);

//-----------------------------------------------------------------------------
bool PdbGetFileSize(const TCHAR* pFileName, DWORD& FileSize) {
  HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, 0, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    ORBIT_ERROR;
    return false;
  }

  // Obtain the size of the file
  FileSize = ::GetFileSize(hFile, NULL);

  if (FileSize == INVALID_FILE_SIZE) {
    ORBIT_ERROR;
    // and continue ...
  }

  // Close the file
  if (!::CloseHandle(hFile)) {
    ORBIT_ERROR;
    // and continue ...
  }

  // Complete
  return (FileSize != INVALID_FILE_SIZE);
}

//-----------------------------------------------------------------------------
bool GetFileParams(const TCHAR* pFileName, uint64_t& BaseAddr,
                   DWORD& FileSize) {
  // Check parameters

  if (pFileName == 0) {
    return false;
  }

  // Determine the extension of the file

  TCHAR szFileExt[_MAX_EXT] = {0};

  /*
  void __cdecl _tsplitpath (
  register const _TSCHAR *path,
  _TSCHAR *drive,
  _TSCHAR *dir,
  _TSCHAR *fname,
  _TSCHAR *ext
  )
  {
  */

  _tsplitpath_s(pFileName, NULL, 0, NULL, 0, NULL, 0, szFileExt, _MAX_EXT);

  // Is it .PDB file ?

  if (_tcsicmp(szFileExt, _T(".PDB")) == 0) {
    // Yes, it is a .PDB file

    // Determine its size, and use a dummy base address

    BaseAddr =
        0x10000000;  // it can be any non-zero value, but if we load symbols
    // from more than one file, memory regions specified
    // for different files should not overlap
    // (region is "base address + file size")

    if (!PdbGetFileSize(pFileName, FileSize)) {
      return false;
    }

  } else {
    // It is not a .PDB file

    // Base address and file size can be 0

    BaseAddr = 0;
    FileSize = 0;
  }

  // Complete

  return true;
}

//-----------------------------------------------------------------------------
void ShowSymbolInfo(IMAGEHLP_MODULE64& ModuleInfo) {
  switch (ModuleInfo.SymType) {
    case SymNone:
      ORBIT_LOG("No symbols available for the module.\n");
      break;
    case SymExport:
      ORBIT_LOG("Loaded symbols: Exports\n");
      break;
    case SymCoff:
      ORBIT_LOG("Loaded symbols: COFF\n");
      break;
    case SymCv:
      ORBIT_LOG("Loaded symbols: CodeView\n");
      break;
    case SymSym:
      ORBIT_LOG("Loaded symbols: SYM\n");
      break;
    case SymVirtual:
      ORBIT_LOG("Loaded symbols: Virtual\n");
      break;
    case SymPdb:
      ORBIT_LOG("Loaded symbols: PDB\n");
      break;
    case SymDia:
      ORBIT_LOG("Loaded symbols: DIA\n");
      break;
    case SymDeferred:
      ORBIT_LOG("Loaded symbols: Deferred\n");
      break;
    default:
      ORBIT_LOG("Loaded symbols: Unknown format.\n");
      break;
  }

  // Image name
  if (wcslen(ModuleInfo.ImageName) > 0) {
    ORBIT_LOG(Format(TEXT("Image name: %s \n"), ModuleInfo.ImageName));
  }

  // Loaded image name
  if (wcslen(ModuleInfo.LoadedImageName) > 0) {
    ORBIT_LOG(
        Format(TEXT("Loaded image name: %s \n"), ModuleInfo.LoadedImageName));
  }

  // Loaded PDB name
  if (wcslen(ModuleInfo.LoadedPdbName) > 0) {
    ORBIT_LOG(Format(TEXT("PDB file name: %s \n"), ModuleInfo.LoadedPdbName));
  }

  // Is debug information unmatched ?
  // (It can only happen if the debug information is contained
  // in a separate file (.DBG or .PDB)
  if (ModuleInfo.PdbUnmatched || ModuleInfo.DbgUnmatched) {
    ORBIT_LOG(Format(_T("Warning: Unmatched symbols. \n")));
  }

  // Line numbers available ?
  ORBIT_LOG(Format(_T("Line numbers: %s \n"), ModuleInfo.LineNumbers
                                                  ? _T("Available")
                                                  : _T("Not available")));

  // Global symbols available ?
  ORBIT_LOG(Format(_T("Global symbols: %s \n"), ModuleInfo.GlobalSymbols
                                                    ? _T("Available")
                                                    : _T("Not available" )));

  // Type information available ?
  ORBIT_LOG(Format(_T("Type information: %s \n"), ModuleInfo.TypeInfo
                                                      ? _T("Available")
                                                      : _T("Not available")));

  // Source indexing available ?
  ORBIT_LOG(Format(_T("Source indexing: %s \n"),
                   ModuleInfo.SourceIndexed ? _T("Yes") : _T("No")));

  // Public symbols available ?
  ORBIT_LOG(Format(_T("Public symbols: %s \n"),
                   ModuleInfo.Publics ? _T("Available") : _T("Not available")));
}

//-----------------------------------------------------------------------------
bool Pdb::LoadLinuxDebugSymbols(const char* a_PdbName) {
  SCOPE_TIMER_LOG("LoadLinuxDebugSymbols");
  std::string external = Path::GetExternalPath();
  std::string tmp =
      Path::GetTmpPath() + "cmd_" + OrbitUtils::GetTimeStamp() + ".txt";
  std::string nmCommand = external + std::string("llvm\\llvm-nm.exe ") +
                          a_PdbName /*+ std::string(" -n")*/;

  const char* tmpname = tmp.data();
  std::string cmd = nmCommand + " > " + tmpname;
  std::system(cmd.c_str());
  std::ifstream file(tmpname, std::ios::in | std::ios::binary);
  std::string result;
  std::string line;
  int numAddedFunctions = 0;
  while (std::getline(file, line)) {
    std::vector<std::string> tokens = Tokenize(line);
    if (tokens.size() == 3) {
      const std::string& address = tokens[0];
      const std::string& symbol = tokens[2];
      Function func;
      func.SetName(symbol);
      func.SetAddress(std::stoull(address, nullptr, 16));
      func.SetPrettyName(symbol);
      func.SetModule(Path::GetFileName(a_PdbName));
      func.SetPdb(this);
      this->AddFunction(func);
      ++numAddedFunctions;
    }
  }
  remove(tmpname);
  return numAddedFunctions > 0;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadPdb(const char* a_PdbName) {
  SCOPE_TIMER_LOG("LOAD PDB");

  m_IsLoading = true;
  m_LoadTimer->Start();

  std::string msg = "pdb:" + std::string(a_PdbName);
  GTcpServer->SendToUiAsync(msg);

  std::string nameStr = a_PdbName;

  std::string extension = ToLower(Path::GetExtension(nameStr));
  if (extension == ".dll") {
    SCOPE_TIMER_LOG("LoadDll Exports");
    ParseDll(nameStr.c_str());
  } else if (extension == ".debug") {
    LoadLinuxDebugSymbols(a_PdbName);
  } else {
    LoadPdbDia();
  }

  ShowSymbolInfo(m_ModuleInfo);
  ProcessData();
  GParams.AddToPdbHistory(a_PdbName);

  m_FinishedLoading = true;
  m_IsLoading = false;

  return true;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadDataFromPdb() {
  DiaManager diaManager;
  IDiaDataSource** dataSource = (IDiaDataSource**)&m_DiaDataSource;
  if (!diaManager.LoadDataFromPdb(s2ws(m_FileName).c_str(), dataSource,
                                  &m_DiaSession, &m_DiaGlobalSymbol)) {
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadPdbDia() {
  if (m_DiaGlobalSymbol) {
    Reserve();
    auto group =
        oqpi_tk::make_parallel_group<oqpi::task_type::waitable>("Fork");
    group->addTask(oqpi_tk::make_task_item("DumpAllFunctions", DumpAllFunctions,
                                           m_DiaGlobalSymbol));
    group->addTask(
        oqpi_tk::make_task_item("DumpTypes", DumpTypes, m_DiaGlobalSymbol));
    group->addTask(oqpi_tk::make_task_item(
        "HookDumpAllGlobals", OrbitDumpAllGlobals, m_DiaGlobalSymbol));
    oqpi_tk::schedule_task(oqpi::task_handle(group)).wait();

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
void Pdb::LoadPdbAsync(const char* a_PdbName,
                       std::function<void()> a_CompletionCallback) {
  m_FileName = a_PdbName;
  m_Name = Path::GetFileName(m_FileName);

  m_LoadingCompleteCallback = a_CompletionCallback;
  m_LoadingThread =
      std::make_unique<std::thread>(&Pdb::LoadPdb, this, m_FileName.c_str());
  m_LoadingThread->detach();
}

//-----------------------------------------------------------------------------
void Pdb::PopulateFunctionMap() {
  m_IsPopulatingFunctionMap = true;

  SCOPE_TIMER_LOG(
      absl::StrFormat("Pdb::PopulateFunctionMap for %s", m_FileName.c_str()));

  for (Function& Function : m_Functions) {
    m_FunctionMap.insert(std::make_pair(Function.Address(), &Function));
  }

  m_IsPopulatingFunctionMap = false;
}

//-----------------------------------------------------------------------------
void Pdb::PopulateStringFunctionMap() {
  m_IsPopulatingFunctionStringMap = true;

  SCOPE_TIMER_LOG(absl::StrFormat("Pdb::PopulateStringFunctionMap for %s",
                                  m_FileName.c_str()));

  {
    SCOPE_TIMER_LOG("Reserving map");
    m_StringFunctionMap.reserve(unsigned(1.5f * (float)m_Functions.size()));
  }

  {
    SCOPE_TIMER_LOG("Hash");

    if (m_Functions.size() > 1000) {
      oqpi_tk::parallel_for_each(
          "StringMap", m_Functions,
          [](Function& a_Function) { a_Function.Hash(); });
    } else {
      for (Function& Function : m_Functions) {
        Function.Hash();
      }
    }
  }

  {
    SCOPE_TIMER_LOG("Map inserts");

    for (Function& Function : m_Functions) {
      m_StringFunctionMap[Function.Hash()] = &Function;
    }
  }

  m_IsPopulatingFunctionStringMap = false;
}

//-----------------------------------------------------------------------------
void Pdb::ApplyPresets() {
  SCOPE_TIMER_LOG(absl::StrFormat("Pdb::ApplyPresets - %s", m_Name.c_str()));

  if (Capture::GSessionPresets) {
    std::string pdbName = Path::GetFileName(m_Name);

    auto it = Capture::GSessionPresets->m_Modules.find(pdbName);
    if (it != Capture::GSessionPresets->m_Modules.end()) {
      SessionModule& a_Module = it->second;

      for (uint64_t hash : a_Module.m_FunctionHashes) {
        PRINT_VAR(hash);
        auto fit = m_StringFunctionMap.find(hash);
        if (fit != m_StringFunctionMap.end()) {
          Function* function = fit->second;
          function->Select();
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromExactAddress(uint64_t a_Address) {
  uint64_t address = a_Address - (uint64_t)GetHModule() + load_bias_;

  if (m_FunctionMap.find(address) != m_FunctionMap.end()) {
    return m_FunctionMap[address];
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromProgramCounter(uint64_t a_Address) {
  uint64_t address = a_Address - (uint64_t)GetHModule() + load_bias_;

  auto it = m_FunctionMap.upper_bound(address);
  if (!m_FunctionMap.empty() && it != m_FunctionMap.begin()) {
    --it;
    Function* func = it->second;
    return func;
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
std::shared_ptr<OrbitDiaSymbol> Pdb::SymbolFromAddress(uint64_t a_Address) {
  std::shared_ptr<OrbitDiaSymbol> symbol = std::make_shared<OrbitDiaSymbol>();

  if (m_DiaSession) {
    DWORD rva = DWORD(a_Address - (uint64_t)GetHModule());
    auto error =
        m_DiaSession->findSymbolByRVA(rva, SymTagFunction, &symbol->m_Symbol);
    if (error == S_OK) {
      return symbol;
    } else {
      PrintLastError();
    }
  }

  return symbol;
}

//-----------------------------------------------------------------------------
bool Pdb::LineInfoFromAddress(uint64_t a_Address, LineInfo& o_LineInfo) {
  if (!m_DiaSession) {
    return false;
  }

  OrbitDiaEnumLineNumbers lineNumbers;
  DWORD rva = DWORD(a_Address - (uint64_t)GetHModule());
  std::wstring fileNameW;
  if (SUCCEEDED(m_DiaSession->findLinesByRVA(rva, 1, &lineNumbers.m_Symbol))) {
    OrbitDiaLineNumber pLineNumber;
    ULONG celt = 0;

    while (SUCCEEDED(lineNumbers->Next(1, &pLineNumber.m_Symbol, &celt)) &&
           celt == 1 && fileNameW.size() == 0) {
      OrbitDiaSourceFile sourceFile;
      if (SUCCEEDED(pLineNumber->get_sourceFile(&sourceFile.m_Symbol))) {
        BSTR fileName;
        if (SUCCEEDED(sourceFile->get_fileName(&fileName))) {
          fileNameW = std::wstring(fileName);
          o_LineInfo.m_Address = a_Address;
          o_LineInfo.m_File = fileName;
          o_LineInfo.m_Line = 0;

          SysFreeString(fileName);
        }

        DWORD lineNumber;
        if (SUCCEEDED(pLineNumber->get_lineNumber(&lineNumber))) {
          o_LineInfo.m_Line = lineNumber;
        }
      }

      pLineNumber.Release();
    }
  }

  return fileNameW.size() > 0;
}

//-----------------------------------------------------------------------------
std::shared_ptr<OrbitDiaSymbol> Pdb::GetDiaSymbolFromId(ULONG a_Id) {
  std::shared_ptr<OrbitDiaSymbol> symbol = std::make_shared<OrbitDiaSymbol>();

  if (m_DiaSession) {
    auto error = m_DiaSession->symbolById(a_Id, &symbol->m_Symbol);
    if (error != S_OK) {
      PrintLastError();
    }
  }

  return symbol;
}

//-----------------------------------------------------------------------------
void Pdb::ProcessData() {
  SCOPE_TIMER_LOG(absl::StrFormat("Pdb::ProcessData for %s", m_Name.c_str()));

  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  auto& functions = Capture::GTargetProcess->GetFunctions();
  auto& globals = Capture::GTargetProcess->GetGlobals();

  functions.reserve(functions.size() + m_Functions.size());

  for (Function& func : m_Functions) {
    func.SetPdb(this);
    functions.push_back(&func);
    GOrbitUnreal.OnFunctionAdded(&func);
  }

  if (GParams.m_FindFileAndLineInfo) {
    SCOPE_TIMER_LOG("Find File and Line info");
    for (Function& func : m_Functions) {
      func.FindFile();
    }
  }

  for (Type& type : m_Types) {
    type.m_Pdb = this;
    Capture::GTargetProcess->AddType(type);
    GOrbitUnreal.OnTypeAdded(&type);
  }

  for (Variable& var : m_Globals) {
    var.m_Pdb = this;
    globals.push_back(&var);
  }

  for (auto& it : m_TypeMap) {
    it.second.m_Pdb = this;
  }

  PopulateFunctionMap();
  PopulateStringFunctionMap();
  // TODO: parallelize: PopulateStringFunctionMap();
}

//-----------------------------------------------------------------------------
void Pdb::Save() {
  std::string fullName = Path::GetCachePath() + GetCachedName();

  SCOPE_TIMER_LOG(absl::StrFormat("Saving %s", fullName.c_str()));

  // ofstream os( fullName, std::ios::binary );
  // cereal::BinaryOutputArchive Ar(os);
  // Ar( *this );
}
