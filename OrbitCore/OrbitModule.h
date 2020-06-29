// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory.h>

#include <string>

#include "BaseTypes.h"
#include "OrbitFunction.h"
#include "SerializationMacros.h"
#include "symbol.pb.h"

class Pdb;

//-----------------------------------------------------------------------------
struct Module {
  Module(){};
  Module(const std::string& file_name, uint64_t address_start,
         uint64_t address_end);

  std::string GetPrettyName();
  bool LoadDebugInfo();
  void LoadSymbols(const ModuleSymbols& module_symbols);
  bool ContainsAddress(uint64_t a_Address) {
    return m_AddressStart <= a_Address && m_AddressEnd > a_Address;
  }
  uint64_t ValidateAddress(uint64_t a_Address);
  void SetLoaded(bool value);
  void SetLoadable(bool value) { loadable_ = value; }
  bool IsLoadable() const { return loadable_; }
  bool IsLoaded() const { return loaded_; }

  std::string m_Name;       // name of the file (without path)
  std::string m_FullName;   // full filename (including path)
  std::string m_PdbName;    // full filename of symbols file
  std::string m_Directory;  // path of the module, without name of the file
  std::string m_PrettyName;
  std::string m_AddressRange;

  std::string m_DebugSignature;  // gnu build id on linux
  HMODULE m_ModuleHandle = 0;
  uint64_t m_AddressStart = 0;
  uint64_t m_AddressEnd = 0;
  uint64_t m_EntryPoint = 0;
  bool m_Selected = false;

  uint64_t m_PdbSize = 0;  // Size in bytes; windows: pdb, linux: module

  mutable std::shared_ptr<Pdb> m_Pdb;

 private:
  bool loadable_ = false;
  bool loaded_ = false;

  friend class TestRemoteMessages;
};
