// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_ORBIT_MODULE_H_
#define ORBIT_CORE_ORBIT_MODULE_H_

#include <memory.h>

#include <string>

#include "Pdb.h"
#include "symbol.pb.h"

struct Module {
  Module() = default;
  Module(const std::string& file_name, uint64_t address_start, uint64_t address_end);

  void LoadSymbols(const orbit_grpc_protos::ModuleSymbols& module_symbols);

  void SetLoaded(bool value) { loaded_ = value; }
  void SetLoadable(bool value) { loadable_ = value; }
  bool IsLoadable() const { return loadable_; }
  bool IsLoaded() const { return loaded_; }

  std::string m_Name;      // name of the file (without path)
  std::string m_FullName;  // full filename (including path)

  std::string m_DebugSignature;  // gnu build id on linux
  uint64_t m_AddressStart = 0;
  uint64_t m_AddressEnd = 0;

  uint64_t m_PdbSize = 0;  // Size in bytes; windows: pdb, linux: module

  mutable std::shared_ptr<Pdb> m_Pdb;

 private:
  bool loadable_ = false;
  bool loaded_ = false;
};

#endif  // ORBIT_CORE_ORBIT_MODULE_H_
