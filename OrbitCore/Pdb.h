//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "OrbitDbgHelp.h"
#include "OrbitSession.h"
#include "OrbitType.h"
#include "Variable.h"

struct IDiaSymbol;
struct IDiaSession;
struct IDiaDataSource;

// TODO: Create a common interface with 2 implementations for Linux and Windows
// or separate this class into 2 differently named ones. It will hopefully
// let us remove ifdefs from here and have implementation separated in different
// .cpp files.

#ifdef _WIN32

class Pdb {
 public:
  explicit Pdb(const char* pdb_name = "");
  Pdb(uint64_t module_address, uint64_t load_bias, std::string file_name,
      std::string module_file_name);
  Pdb(const Pdb&) = delete;
  Pdb& operator=(const Pdb&) = delete;
  virtual ~Pdb();

  void Init();

  virtual bool LoadPdb(const char* a_PdbName);
  virtual void LoadPdbAsync(const char* a_PdbName,
                            std::function<void()> a_CompletionCallback);

  bool LoadDataFromPdb();
  bool LoadPdbDia();
  void Update();
  void AddFunction(const std::shared_ptr<Function>& function);
  void CheckOrbitFunction(Function& a_Function);
  void AddType(const Type& a_Type);
  void AddGlobal(const Variable& a_Global);
  void PrintFunction(Function& a_Func);
  void OnReceiveMessage(const Message& a_Msg);
  void AddArgumentRegister(const std::string& a_Reg,
                           const std::string& a_Function);

  const std::string& GetName() const { return m_Name; }
  const std::string& GetFileName() const { return m_FileName; }
  const std::string& GetLoadedModuleName() const { return m_LoadedModuleName; }
  const std::vector<std::shared_ptr<Function>>& GetFunctions() {
    return functions_;
  }
  std::vector<Type>& GetTypes() { return m_Types; }
  std::vector<Variable>& GetGlobals() { return m_Globals; }
  uint64_t GetHModule() const { return m_MainModule; }
  uint64_t GetLoadBias() const { return load_bias_; }
  Type& GetTypeFromId(ULONG a_Id) { return m_TypeMap[a_Id]; }
  Type* GetTypePtrFromId(ULONG a_ID);

  GUID GetGuid();

  void SetMainModule(uint64_t a_Module) { m_MainModule = a_Module; }
  void SetLoadBias(uint64_t load_bias) { load_bias_ = load_bias; }

  void Print() const;
  void PrintGlobals() const;
  void PopulateFunctionMap();
  void PopulateStringFunctionMap();
  void Clear();
  void Reserve();
  void ApplyPresets(const Session& session);

  Function* GetFunctionFromExactAddress(uint64_t a_Address);
  Function* GetFunctionFromProgramCounter(uint64_t a_Address);
  std::shared_ptr<OrbitDiaSymbol> SymbolFromAddress(uint64_t a_Address);
  bool LineInfoFromAddress(uint64_t a_Address, struct LineInfo& o_LineInfo);

  void SetLoadTime(float a_LoadTime) { m_LastLoadTime = a_LoadTime; }
  float GetLoadTime() { return m_LastLoadTime; }

  std::string GetCachedName();
  std::string GetCachedKey();
  bool Load(const std::string& a_CachedPdb);
  void Save();

  bool IsLoading() const { return m_IsLoading; }

  template <class Archive>
  void serialize(Archive& ar, std::uint32_t const version) {
    /*ar( CEREAL_NVP(m_Name)
      , CEREAL_NVP(m_FileName)
      , CEREAL_NVP(m_FileNameW)
      , CEREAL_NVP(m_Functions)
      , CEREAL_NVP(m_Types)
      , CEREAL_NVP(m_Globals)
      , CEREAL_NVP(m_ModuleInfo)
      , CEREAL_NVP(m_TypeMap) );*/
  }

  std::shared_ptr<OrbitDiaSymbol> GetDiaSymbolFromId(ULONG a_Id);
  void ProcessData();

 protected:
  void SendStatusToUi();

 protected:
  // State
  std::unique_ptr<std::thread> m_LoadingThread;
  std::atomic<bool> m_FinishedLoading;
  std::atomic<bool> m_IsLoading;
  std::atomic<bool> m_IsPopulatingFunctionMap;
  std::atomic<bool> m_IsPopulatingFunctionStringMap;
  std::function<void()> m_LoadingCompleteCallback;
  uint64_t m_MainModule;
  uint64_t load_bias_ = 0;
  float m_LastLoadTime;
  bool m_LoadedFromCache;
  std::vector<Variable> m_WatchedVariables;
  std::set<std::string> m_ArgumentRegisters;
  std::map<std::string, std::vector<std::string>> m_RegFunctionsMap;

  // Data
  std::string m_Name;
  std::string m_FileName;
  std::string m_LoadedModuleName;
  std::vector<std::shared_ptr<Function>> functions_;
  std::vector<Type> m_Types;
  std::vector<Variable> m_Globals;
  IMAGEHLP_MODULE64 m_ModuleInfo;
  std::unordered_map<ULONG, Type> m_TypeMap;
  std::map<uint64_t, Function*> m_FunctionMap;
  std::unordered_map<unsigned long long, Function*> m_StringFunctionMap;
  Timer* m_LoadTimer;

  // DIA
  IDiaSession* m_DiaSession = nullptr;
  IDiaSymbol* m_DiaGlobalSymbol = nullptr;
  IDiaDataSource* m_DiaDataSource = nullptr;
};

#else
class Pdb {
 public:
  Pdb() = default;
  Pdb(uint64_t module_address, uint64_t load_bias, std::string file_name,
      std::string module_file_name);
  Pdb(const Pdb&) = delete;
  Pdb& operator=(const Pdb&) = delete;
  virtual ~Pdb() = default;

  void Init() {}

  bool LoadPdb(const char* /*file_name*/) {
    return false;  // Should not do anything on linux
  }
  virtual void LoadPdbAsync(const char* pdb_name,
                            std::function<void()> completion_callback);

  bool LoadDataFromPdb() {
    return true;
  }  // This shouldn't do anything on Linux.
  bool LoadPdbDia() { return false; }
  void Update() {}
  void CheckOrbitFunction(Function&) {}
  void AddType(const Type&) {}
  void AddGlobal(const Variable&) {}
  void PrintFunction(Function&) {}
  void OnReceiveMessage(const Message&) {}
  void AddArgumentRegister(const std::string&, const std::string&) {}

  const std::string& GetName() const { return m_Name; }
  const std::string& GetFileName() const { return m_FileName; }
  const std::string& GetLoadedModuleName() const { return m_LoadedModuleName; }
  const std::vector<std::shared_ptr<Function>>& GetFunctions() {
    return functions_;
  }
  void AddFunction(const std::shared_ptr<Function>& function);
  std::vector<Type>& GetTypes() { return m_Types; }
  std::vector<Variable>& GetGlobals() { return m_Globals; }
  uint64_t GetHModule() const { return m_MainModule; }
  uint64_t GetLoadBias() const { return load_bias_; }
  Type& GetTypeFromId(ULONG a_Id) { return m_TypeMap[a_Id]; }
  Type* GetTypePtrFromId(ULONG a_ID);

  GUID GetGuid();

  void SetMainModule(uint64_t a_Module) { m_MainModule = a_Module; }
  void SetLoadBias(uint64_t load_bias) { load_bias_ = load_bias; }

  void Print() const;
  void PrintGlobals() const;
  void PopulateFunctionMap();
  void PopulateStringFunctionMap();
  void Clear();
  void Reserve();
  void ApplyPresets(const Session& session);

  Function* GetFunctionFromExactAddress(uint64_t a_Address);
  Function* GetFunctionFromProgramCounter(uint64_t a_Address);
  IDiaSymbol* SymbolFromAddress(uint64_t a_Address);
  bool LineInfoFromAddress(uint64_t a_Address, struct LineInfo& o_LineInfo);
  Function* FunctionFromName(const std::string& a_Name);

  void SetLoadTime(float a_LoadTime) { m_LastLoadTime = a_LoadTime; }
  float GetLoadTime() { return m_LastLoadTime; }

  std::string GetCachedName();
  std::string GetCachedKey();
  bool Load(const std::string& a_CachedPdb);
  void Save();

  bool IsLoading() const { return m_IsLoading; }

  template <class Archive>
  void serialize(Archive&, uint32_t /*version*/) {}

  IDiaSymbol* GetDiaSymbolFromId(ULONG a_Id);
  void ProcessData();

 protected:
  void SendStatusToUi();

 protected:
  // State
  std::unique_ptr<std::thread> m_LoadingThread;
  std::atomic<bool> m_FinishedLoading;
  std::atomic<bool> m_IsLoading;
  std::function<void()> m_LoadingCompleteCallback;
  uint64_t m_MainModule = 0;
  uint64_t load_bias_ = 0;
  float m_LastLoadTime = 0;
  std::vector<Variable> m_WatchedVariables;
  std::set<std::string> m_ArgumentRegisters;
  std::map<std::string, std::vector<std::string>> m_RegFunctionsMap;

  // Data
  std::string m_Name;              // name of the file containing the symbols
  std::string m_FileName;          // full path of file containing the symbols
  std::string m_LoadedModuleName;  // full path of the module
  std::vector<std::shared_ptr<Function>> functions_;
  std::vector<Type> m_Types;
  std::vector<Variable> m_Globals;
  std::unordered_map<ULONG, Type> m_TypeMap;
  std::map<uint64_t, Function*> m_FunctionMap;
  std::unordered_map<unsigned long long, Function*> m_StringFunctionMap;
  Timer* m_LoadTimer = nullptr;
};
#endif

extern std::shared_ptr<Pdb> GPdbDbg;
