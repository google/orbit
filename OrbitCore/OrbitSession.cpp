// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "OrbitSession.h"

//-----------------------------------------------------------------------------
Session::Session() {}

//-----------------------------------------------------------------------------
Session::~Session() {}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(Session, 1) {
  ORBIT_NVP_VAL(0, m_ProcessFullPath);
  ORBIT_NVP_VAL(0, m_Modules);
  ORBIT_NVP_VAL(1, m_WorkingDirectory);
  ORBIT_NVP_VAL(1, m_Arguments);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(SessionModule, 0) {
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_FunctionHashes);
}
