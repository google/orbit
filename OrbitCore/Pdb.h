// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_PDB_H_
#define ORBIT_CORE_PDB_H_

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "capture_data.pb.h"
#include "preset.pb.h"

class Pdb {
 public:
  Pdb(uint64_t module_address, uint64_t load_bias, std::string file_name,
      std::string module_file_name);
  Pdb(const Pdb&) = delete;
  Pdb& operator=(const Pdb&) = delete;

  const std::string& GetName() const { return m_Name; }
  const std::string& GetFileName() const { return m_FileName; }
  const std::string& GetLoadedModuleName() const { return m_LoadedModuleName; }
  const std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>&
  GetFunctions() {
    return functions_;
  }
  void AddFunction(
      const std::shared_ptr<orbit_client_protos::FunctionInfo>& function);
  uint64_t GetHModule() const { return m_MainModule; }
  uint64_t GetLoadBias() const { return load_bias_; }

  void PopulateFunctionMap();
  void PopulateStringFunctionMap();
  [[nodiscard]] std::vector<orbit_client_protos::FunctionInfo*>
  FunctionsToSelect(const orbit_client_protos::PresetFile& preset) const;

  orbit_client_protos::FunctionInfo* GetFunctionFromExactAddress(
      uint64_t a_Address);
  orbit_client_protos::FunctionInfo* GetFunctionFromProgramCounter(
      uint64_t a_Address);

  void ProcessData();

 private:
  uint64_t m_MainModule = 0;
  uint64_t load_bias_ = 0;
  std::string m_Name;              // name of the file containing the symbols
  std::string m_FileName;          // full path of file containing the symbols
  std::string m_LoadedModuleName;  // full path of the module
  std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>> functions_;
  std::map<uint64_t, orbit_client_protos::FunctionInfo*> m_FunctionMap;
  std::unordered_map<unsigned long long, orbit_client_protos::FunctionInfo*>
      m_StringFunctionMap;
};

#endif  // ORBIT_CORE_PDB_H_
