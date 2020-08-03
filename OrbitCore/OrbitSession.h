// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "Core.h"
#include "Serialization.h"
#include "preset.pb.h"

class Preset {
 public:
  Preset();
  ~Preset();

  ORBIT_SERIALIZABLE;

  std::string m_FileName;
  std::string m_ProcessFullPath;
  std::map<std::string, orbit_client_protos::PresetModule> m_Modules;
};
