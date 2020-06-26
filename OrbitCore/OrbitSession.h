// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include "Core.h"
#include "Serialization.h"

//-----------------------------------------------------------------------------
struct PresetModule {
  std::string m_Name;
  std::vector<uint64_t> m_FunctionHashes;

  ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
class Preset {
 public:
  Preset();
  ~Preset();

  ORBIT_SERIALIZABLE;

  std::string m_FileName;
  std::string m_ProcessFullPath;
  std::string m_WorkingDirectory;
  std::string m_Arguments;
  std::map<std::string, PresetModule> m_Modules;
};
