//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "BaseTypes.h"
#include "FunctionStats.h"
#include "OrbitDbgHelp.h"
#include "SerializationMacros.h"
#include "Utils.h"
#include "cvconst.h"

#ifdef _WIN32
#include "TypeInfoStructs.h"
#endif

class Pdb;
class Type;

struct FunctionParam {
  FunctionParam();
  std::string m_Name;
  std::string m_ParamType;
  std::string m_Type;
  std::string m_Address;

#ifdef _WIN32
  SYMBOL_INFO m_SymbolInfo;
#endif

  bool InRegister(int a_Index);
  bool IsPointer() { return m_Type.find("*") != std::string::npos; }
  bool IsRef() { return m_Type.find("&") != std::string::npos; }
  bool IsFloat();
};

struct Argument {
  Argument() { memset(this, 0, sizeof(*this)); }
  DWORD m_Index;
  CV_HREG_e m_Reg;
  DWORD m_Offset;
  DWORD m_NumBytes;
};

struct FunctionArgInfo {
  FunctionArgInfo() : m_NumStackBytes(0), m_ArgDataSize(0) {}
  int m_NumStackBytes;
  int m_ArgDataSize;
  std::vector<Argument> m_Args;
};

class Function {
 public:
  enum OrbitType {
    NONE,
    ORBIT_TIMER_START,
    ORBIT_TIMER_STOP,
    ORBIT_LOG,
    ORBIT_OUTPUT_DEBUG_STRING,
    UNREAL_ACTOR,
    ALLOC,
    FREE,
    REALLOC,
    ORBIT_DATA,
    NUM_TYPES
  };

  Function();
  // TODO: remove references to Pdb from the function, it likely just needs
  // the module base address and load_bias.
  Function(std::string_view name, std::string_view pretty_name,
           std::string_view file, uint64_t address, uint64_t size,
           uint64_t load_bias, Pdb* pdb);

  void Print();
  void SetAsMainFrameFunction();
  void AddParameter(const FunctionParam& param) { params_.push_back(param); }

  // TODO: It looks like most setters are used by TestRemoteMessages::Run()
  // only. Move these to a constructor?
  void SetName(const std::string& name) { name_ = name; }
  void SetPrettyName(const std::string& pretty_name) {
    pretty_name_ = pretty_name;
  }
  void SetAddress(uint64_t address) { address_ = address; }
  void SetModule(const std::string& module) { module_ = module; }
  void SetFile(const std::string& file) { file_ = file; }
  void SetProbe(const std::string& probe) { probe_ = probe; }
  void SetId(uint32_t id) { id_ = id; }
  void SetParentId(uint32_t parent_id) { parent_id_ = parent_id; }
  void SetSize(uint64_t size) { size_ = size; }
  void SetLine(uint32_t line) { line_ = line; }
  void SetCallingConvention(int calling_convention) {
    calling_convention_ = calling_convention;
  }
  void SetOrbitType(OrbitType type) { type_ = type; }
  void SetPdb(Pdb* pdb);

  const std::string& Name() const { return name_; }
  const std::string& PrettyName() const;
  const std::string& Lower() {
    if (pretty_name_lower_.empty()) {
      pretty_name_lower_ = ToLower(pretty_name_);
    }
    return pretty_name_lower_;
  }
  OrbitType GetOrbitType() const { return type_; }
  uint32_t Line() const { return line_; }
  uint64_t Size() const { return size_; }
  const std::string& Probe() const { return probe_; }
  int CallingConvention() const { return calling_convention_; }
  const Pdb* GetPdb() const { return pdb_; }
  const FunctionStats& Stats() const { return *stats_; }
  const char* GetCallingConventionString();
  void ProcessArgumentInfo();
  bool IsMemberFunction();
  uint64_t Hash() const { return StringHash(pretty_name_); }
  void UpdateStats(const Timer& timer);
  bool Hookable();
  void Select();
  void UnSelect();
  void ToggleSelect() { /*if( Hookable() )*/
    selected_ = !selected_;
  }
  bool IsSelected() const { return selected_; }
  // Calculates and returns the absolute address of the function.
  uint64_t GetVirtualAddress() const;
  uint64_t Address() const { return address_; }
  uint64_t Offset() const;
  const std::string& File() const { return file_; }
  bool IsOrbitFunc() const { return type_ != OrbitType::NONE; }
  bool IsOrbitZone() const {
    return type_ == ORBIT_TIMER_START || type_ == ORBIT_TIMER_STOP;
  }
  bool IsOrbitStart() const { return type_ == ORBIT_TIMER_START; }
  bool IsOrbitStop() const { return type_ == ORBIT_TIMER_STOP; }
  bool IsRealloc() const { return type_ == REALLOC; }
  bool IsAlloc() const { return type_ == ALLOC; }
  bool IsFree() const { return type_ == FREE; }
  bool IsMemoryFunc() const { return IsFree() || IsAlloc() || IsRealloc(); }
  std::string GetModuleName() const;
  const std::string& GetLoadedModuleName() const { return loaded_module_name_; }
  Type* GetParentType();
  void ResetStats();
  void GetDisassembly(uint32_t pid);
  void FindFile();

  ORBIT_SERIALIZABLE;

 private:
  std::string name_;
  std::string pretty_name_;
  std::string pretty_name_lower_;
  std::string loaded_module_name_;
  std::string module_;
  std::string file_;
  std::string probe_;
  uint64_t address_ = 0;
  uint64_t size_ = 0;
  uint64_t load_bias_ = 0;
  uint64_t module_base_address_ = 0;
  uint32_t id_ = 0;
  uint32_t parent_id_ = 0;
  uint32_t line_ = 0;
  int calling_convention_ = -1;
  std::vector<FunctionParam> params_;
  std::vector<Argument> arguments_;
  Pdb* pdb_ = nullptr;
  OrbitType type_ = NONE;
  std::shared_ptr<FunctionStats> stats_;
  bool selected_ = false;
};
