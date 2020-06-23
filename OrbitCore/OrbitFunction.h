// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "BaseTypes.h"
#include "FunctionStats.h"
#include "OrbitDbgHelp.h"
#include "Path.h"
#include "SerializationMacros.h"
#include "Utils.h"
#include "cvconst.h"

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

  Function() : Function{"", "", 0, 0, 0, "", 0} {}

  Function(std::string_view name, std::string_view pretty_name,
           uint64_t address, uint64_t load_bias, uint64_t size,
           std::string_view file, uint32_t line);

  const std::string& Name() const { return name_; }
  const std::string& PrettyName() const {
    return pretty_name_.empty() ? name_ : pretty_name_;
  }
  std::string Lower() { return ToLower(PrettyName()); }
  const std::string& GetLoadedModulePath() const { return loaded_module_path_; }
  std::string GetLoadedModuleName() const {
    return Path::GetFileName(loaded_module_path_);
  }
  void SetModulePathAndAddress(std::string_view module_path,
                               uint64_t module_adddress) {
    loaded_module_path_ = module_path;
    module_base_address_ = module_adddress;
  }
  uint64_t Size() const { return size_; }
  const std::string& File() const { return file_; }
  uint32_t Line() const { return line_; }
  int CallingConvention() const { return calling_convention_; }
  const char* GetCallingConventionString();
  void SetCallingConvention(int calling_convention) {
    calling_convention_ = calling_convention;
  }

  uint64_t Hash() const { return StringHash(pretty_name_); }

  // Calculates and returns the absolute address of the function.
  uint64_t Address() const { return address_; }
  uint64_t Offset() const { return address_ - load_bias_; }
  uint64_t GetVirtualAddress() const {
    return address_ + module_base_address_ - load_bias_;
  }

  OrbitType GetOrbitType() const { return type_; }
  void SetOrbitType(OrbitType type) { type_ = type; }
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

  const FunctionStats& GetStats() const { return *stats_; }
  void UpdateStats(const Timer& timer);
  void ResetStats();

  bool Hookable();
  void Select();
  void UnSelect();
  bool IsSelected() const;

  void SetId(uint32_t id) { id_ = id; }
  void SetParentId(uint32_t parent_id) { parent_id_ = parent_id; }
  void AddParameter(const FunctionParam& param) { params_.push_back(param); }
  void Print();

  void GetDisassembly(uint32_t pid);
  void FindFile();

  ORBIT_SERIALIZABLE;

 private:
  std::string name_;
  std::string pretty_name_;
  std::string loaded_module_path_;
  uint64_t module_base_address_ = 0;
  uint64_t address_;
  uint64_t load_bias_;
  uint64_t size_;
  std::string file_;
  uint32_t line_;
  uint32_t id_ = 0;
  uint32_t parent_id_ = 0;
  int calling_convention_ = -1;
  std::vector<FunctionParam> params_;
  std::vector<Argument> arguments_;
  OrbitType type_ = NONE;
  std::shared_ptr<FunctionStats> stats_;
};
