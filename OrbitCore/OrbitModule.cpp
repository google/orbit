//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitModule.h"

#include <string>

#include "Core.h"
#include "Pdb.h"
#include "Serialization.h"
#include "absl/strings/str_format.h"

#ifndef WIN32
#include "Capture.h"
#include "ElfFile.h"
#include "LinuxUtils.h"
#include "OrbitProcess.h"
#include "OrbitUnreal.h"
#include "Params.h"
#include "Path.h"
#include "ScopeTimer.h"
#endif

//-----------------------------------------------------------------------------
Module::Module() { m_Pdb = std::make_shared<Pdb>(); }

//-----------------------------------------------------------------------------
std::string Module::GetPrettyName() {
  if (m_PrettyName.size() == 0) {
#ifdef WIN32
    m_PrettyName =
        absl::StrFormat("%s [%I64x - %I64x] %s\r\n", m_Name.c_str(),
                        m_AddressStart, m_AddressEnd, m_FullName.c_str());
    m_AddressRange =
        absl::StrFormat("[%I64x - %I64x]", m_AddressStart, m_AddressEnd);
#else
    m_PrettyName = m_FullName;
    m_AddressRange =
        absl::StrFormat("[%016llx - %016llx]", m_AddressStart, m_AddressEnd);
    m_PdbName = m_FullName;
    m_FoundPdb = true;
#endif
  }

  return m_PrettyName;
}

//-----------------------------------------------------------------------------
bool Module::IsDll() const {
  return ToLower(Path::GetExtension(m_FullName)) == std::string(".dll") ||
         Contains(m_Name, ".so");
}

//-----------------------------------------------------------------------------
bool Module::LoadDebugInfo() {
  assert(m_Pdb);
  m_Pdb->SetMainModule(m_AddressStart);

  PRINT_VAR(m_FoundPdb);
  if (m_FoundPdb) {
    return m_Pdb->LoadDataFromPdb();
  }

  return false;
}

//-----------------------------------------------------------------------------
uint64_t Module::ValidateAddress(uint64_t a_Address) {
  if (ContainsAddress(a_Address)) return a_Address;

  // Treat input address as RVA
  uint64_t newAddress = m_AddressStart + a_Address;
  if (ContainsAddress(newAddress)) return newAddress;

  return 0xbadadd;
}

//-----------------------------------------------------------------------------
void Module::SetLoaded(bool a_Value) { m_Loaded = a_Value; }

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(Module, 0) {
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_FullName);
  ORBIT_NVP_VAL(0, m_PdbName);
  ORBIT_NVP_VAL(0, m_Directory);
  ORBIT_NVP_VAL(0, m_PrettyName);
  ORBIT_NVP_VAL(0, m_AddressRange);
  ORBIT_NVP_VAL(0, m_DebugSignature);
  ORBIT_NVP_VAL(0, m_AddressStart);
  ORBIT_NVP_VAL(0, m_AddressEnd);
  ORBIT_NVP_VAL(0, m_EntryPoint);
  ORBIT_NVP_VAL(0, m_FoundPdb);
  ORBIT_NVP_VAL(0, m_Selected);
  ORBIT_NVP_VAL(0, m_Loaded);
  ORBIT_NVP_VAL(0, m_PdbSize);
}

#ifndef WIN32

//-----------------------------------------------------------------------------
Function* Pdb::FunctionFromName(const std::string& a_Name) {
  uint64_t hash = StringHash(a_Name);
  auto iter = m_StringFunctionMap.find(hash);
  return (iter == m_StringFunctionMap.end()) ? nullptr : iter->second;
}

//-----------------------------------------------------------------------------
std::unique_ptr<ElfFile> FindSymbols(const std::string& module_path) {
  std::unique_ptr<ElfFile> module_elf_file = ElfFile::Create(module_path);
  if (module_elf_file == nullptr) {
    PRINT(absl::StrFormat("Unable to load module (object file) from \"%s\"\n",
                          module_path));
    return nullptr;
  }

  if (module_elf_file->HasSymtab()) {
    return module_elf_file;
  }

  // Look for .debug files associated with passed in module name
  // TODO: .debug file might not have same name as module, maybe there is a good
  //       way of searching files based on buildId

  std::string dir = Path::GetDirectory(module_path);
  std::string file = Path::StripExtension(Path::GetFileName(module_path));

  std::vector<std::string> search_directories = {
      dir, dir + "debug_symbols/", Path::GetHome(), "/home/cloudcast/"};
  std::vector<std::string> symbol_file_extensions = {".debug", ".elf.debug"};

  std::vector<std::string> search_file_paths;
  for (const auto& directory : search_directories) {
    for (const auto& file_extension : symbol_file_extensions) {
      search_file_paths.emplace_back(directory + file + file_extension);
    }
  }

  for (const auto& symbols_file_path : search_file_paths) {
    if (!Path::FileExists(symbols_file_path)) continue;

    std::unique_ptr<ElfFile> symbols_file = ElfFile::Create(symbols_file_path);
    if (symbols_file == nullptr) continue;

    if (!symbols_file->HasSymtab()) continue;

    std::string module_build_id = module_elf_file->GetBuildId();
    if (module_build_id != "" &&
        module_build_id == symbols_file->GetBuildId()) {
      return symbols_file;
    }
  }

  return nullptr;
}

bool Pdb::LoadFunctions(const char* file_name) {
  m_LoadedModuleName = file_name;
  std::unique_ptr<ElfFile> elf_file = FindSymbols(file_name);

  if (elf_file == nullptr) {
    PRINT(absl::StrFormat("Unable to load elf-file \"%s\"", file_name));
    return false;
  }

  m_FileName = elf_file->GetFilePath();
  m_Name = Path::GetFileName(m_FileName);

  auto load_bias = elf_file->GetLoadBias();
  if (!load_bias) {
    PRINT(absl::StrFormat("Unable to get load_bias \"%s\" "
                          "(does the file have PT_LOAD program headers?)",
                          m_FileName));
    return false;
  }

  load_bias_ = load_bias.value();

  if (!elf_file->GetFunctions(this, &m_Functions)) {
    PRINT(absl::StrFormat("Unable to load functions from \"%s\"", m_FileName));
    return false;
  }

  // For uprobes we need a function to be in the .text segment (why?)
  // TODO: Shouldn't m_Functions be limited to the list of functions referencing
  // .text segment?
  for (auto& function : m_Functions) {
    // Check if function is in .text segment
    if (!elf_file->IsAddressInTextSection(function.Address())) {
      continue;
    }

    function.SetProbe(m_FileName + ":" + function.Name());
  }

  return true;
}

bool Pdb::LoadPdb(const char* file_name) {
  if (!LoadFunctions(file_name)) {
    return false;
  }

  ProcessData();

  return true;
}

//-----------------------------------------------------------------------------
void Pdb::LoadPdbAsync(const char* a_PdbName,
                       std::function<void()> a_CompletionCallback) {
  m_LoadingCompleteCallback = a_CompletionCallback;
  LoadPdb(a_PdbName);
  a_CompletionCallback();
}

//-----------------------------------------------------------------------------
void Pdb::ProcessData() {
  SCOPE_TIMER_LOG("ProcessData");
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
}

//-----------------------------------------------------------------------------
void Pdb::PopulateFunctionMap() {
  SCOPE_TIMER_LOG("Pdb::PopulateFunctionMap");
  for (Function& function : m_Functions) {
    m_FunctionMap.insert(std::make_pair(function.Address(), &function));
  }
}

//-----------------------------------------------------------------------------
void Pdb::PopulateStringFunctionMap() {
  {
    // SCOPE_TIMER_LOG("Reserving map");
    m_StringFunctionMap.reserve(unsigned(1.5f * (float)m_Functions.size()));
  }

  {
    // SCOPE_TIMER_LOG("Map inserts");
    for (Function& Function : m_Functions) {
      m_StringFunctionMap[Function.Hash()] = &Function;
    }
  }
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromExactAddress(uint64_t a_Address) {
  uint64_t function_address = a_Address - (uint64_t)GetHModule() + load_bias_;
  auto it = m_FunctionMap.find(function_address);
  return (it != m_FunctionMap.end()) ? it->second : nullptr;
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromProgramCounter(uint64_t a_Address) {
  if (m_FunctionMap.empty()) {
    return nullptr;
  }

  uint64_t relative_address = a_Address - (uint64_t)GetHModule() + load_bias_;
  auto it = m_FunctionMap.upper_bound(relative_address);

  if (it == m_FunctionMap.begin()) {
    return nullptr;
  }

  --it;
  return it->second;
}

#endif

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(ModuleDebugInfo, 0) {
  ORBIT_NVP_VAL(0, m_Pid);
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_Functions);
  ORBIT_NVP_VAL(0, load_bias);
}
