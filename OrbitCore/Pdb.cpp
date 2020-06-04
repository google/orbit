// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Pdb.h"

#include <ppl.h>

#include <algorithm>
#include <fstream>
#include <functional>

#include "Capture.h"
#include "Core.h"
#include "Log.h"
#include "ObjectCount.h"
#include "OrbitSession.h"
#include "OrbitType.h"
#include "OrbitUnreal.h"
#include "Params.h"
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
      m_IsPopulatingFunctionStringMap(false) {
  m_Name = Path::GetFileName(m_FileName);
  memset(&m_ModuleInfo, 0, sizeof(IMAGEHLP_MODULE64));
  m_ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
  m_LoadTimer = new Timer();
}

//-----------------------------------------------------------------------------
Pdb::Pdb(uint64_t module_address, uint64_t load_bias, std::string file_name,
         std::string module_file_name)
    : m_MainModule(module_address),
      load_bias_(load_bias),
      m_FileName(std::move(file_name)),
      m_LoadedModuleName(std::move(module_file_name)) {
  m_Name = Path::GetFileName(m_FileName);
}

//-----------------------------------------------------------------------------
Pdb::~Pdb() {}

//-----------------------------------------------------------------------------
void Pdb::AddFunction(const std::shared_ptr<Function>& function) {
  functions_.push_back(function);
  functions_.back()->SetModulePathAndAddress(GetLoadedModuleName(),
                                             GetHModule());
  CheckOrbitFunction(*functions_.back());
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
  return guid;
}

//-----------------------------------------------------------------------------
void Pdb::PrintFunction(Function& a_Function) { a_Function.Print(); }

//-----------------------------------------------------------------------------
void Pdb::Clear() {
  functions_.clear();
  m_Types.clear();
  m_Globals.clear();
  m_TypeMap.clear();
  m_FunctionMap.clear();
  m_FileName = "";
}

//-----------------------------------------------------------------------------
void Pdb::Reserve() {
  const int size = 8 * 1024;
  functions_.reserve(size);
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
  std::string status = absl::StrFormat(
      "status:Parsing %s\nFunctions: %i\nTypes: %i\nGlobals: %i\n", m_Name,
      functions_.size(), m_Types.size(), m_Globals.size());

  if (m_IsPopulatingFunctionMap) {
    status += "PopulatingFunctionMap\n";
  }
  if (m_IsPopulatingFunctionStringMap) {
    status += "PopulatingFunctionStringMap\n";
  }

  int numPoints = 10;
  int period = 4000;
  int progress = (int)(float(((ULONG)m_LoadTimer->QueryMillis() % period)) /
                       (float)period * (float)numPoints);
  if (progress > numPoints) progress = numPoints;
  for (int i = 0; i <= progress; ++i) {
    status += ".";
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
  } else {
    LoadPdbDia();
  }

  ProcessData();
  GParams.AddToPdbHistory(a_PdbName);

  m_FinishedLoading = true;
  m_IsLoading = false;
  return true;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadDataFromPdb() {
  // TODO(b/158093728): Dia Loading was disabled, reimplement using LLVM.
  return false;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadPdbDia() {
  // TODO(b/158093728): Dia Loading was disabled, reimplement using LLVM.
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

  for (auto& function : functions_) {
    m_FunctionMap.insert(std::make_pair(function->Address(), function.get()));
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
    m_StringFunctionMap.reserve(unsigned(1.5f * (float)functions_.size()));
  }

  {
    SCOPE_TIMER_LOG("Hash");

    if (functions_.size() > 1000) {
      oqpi_tk::parallel_for_each(
          "StringMap", functions_,
          [](std::shared_ptr<Function>& a_Function) { a_Function->Hash(); });
    } else {
      for (std::shared_ptr<Function>& function : functions_) {
        function->Hash();
      }
    }
  }

  {
    SCOPE_TIMER_LOG("Map inserts");

    for (std::shared_ptr<Function>& function : functions_) {
      m_StringFunctionMap[function->Hash()] = function.get();
    }
  }

  m_IsPopulatingFunctionStringMap = false;
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
bool Pdb::LineInfoFromAddress(uint64_t /*a_Address*/,
                              LineInfo& /*o_LineInfo*/) {
  // TODO(b/158093728): Dia Loading was disabled, reimplement using LLVM.
  return false;
}

//-----------------------------------------------------------------------------
void Pdb::ProcessData() {
  if (!Capture::GTargetProcess) return;

  SCOPE_TIMER_LOG(absl::StrFormat("Pdb::ProcessData for %s", m_Name.c_str()));

  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());

  auto& globals = Capture::GTargetProcess->GetGlobals();

  for (auto& func : functions_) {
    func->SetModulePathAndAddress(GetLoadedModuleName(), GetHModule());
    Capture::GTargetProcess->AddFunction(func);
    GOrbitUnreal.OnFunctionAdded(func.get());
  }

  if (GParams.m_FindFileAndLineInfo) {
    SCOPE_TIMER_LOG("Find File and Line info");
    for (auto& func : functions_) {
      func->FindFile();
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
