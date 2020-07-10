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
#include "absl/container/flat_hash_map.h"

class Pdb;

class Function {
 public:
  // TODO enum class instead of enum
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
    ORBIT_TIMER_START_ASYNC,
    ORBIT_TIMER_STOP_ASYNC,
    ORBIT_TRACK_INT,
    ORBIT_TRACK_INT_64,
    ORBIT_TRACK_UINT,
    ORBIT_TRACK_UINT_64,
    ORBIT_TRACK_FLOAT,
    ORBIT_TRACK_DOUBLE,
    ORBIT_TRACK_FLOAT_AS_INT,
    ORBIT_TRACK_DOUBLE_AS_INT_64,
    // Append new types here.

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

  void SetOrbitType(OrbitType type) { type_ = type; }
  bool SetOrbitTypeFromName();
  bool IsOrbitFunc() const { return type_ != OrbitType::NONE; }

  const FunctionStats& GetStats() const { return *stats_; }
  void UpdateStats(const Timer& timer);
  void ResetStats();

  void Select();
  void UnSelect();
  bool IsSelected() const;

  void Print();

  ORBIT_SERIALIZABLE;

 private:
  static const absl::flat_hash_map<const char*, OrbitType>&
  GetFunctionNameToOrbitTypeMap();
  std::string name_;
  std::string pretty_name_;
  std::string loaded_module_path_;
  uint64_t module_base_address_ = 0;
  uint64_t address_;
  uint64_t load_bias_;
  uint64_t size_;
  std::string file_;
  uint32_t line_;
  int calling_convention_ = -1;
  OrbitType type_ = NONE;
  std::shared_ptr<FunctionStats> stats_;
};
