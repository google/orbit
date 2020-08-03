// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSession.h"

//-----------------------------------------------------------------------------
Preset::Preset() {}

//-----------------------------------------------------------------------------
Preset::~Preset() {}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(Preset, 1) {
  ORBIT_NVP_VAL(0, m_ProcessFullPath);
  ORBIT_NVP_VAL(0, m_Modules);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(PresetModule, 0) {
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_FunctionHashes);
}
