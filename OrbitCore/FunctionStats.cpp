// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionStats.h"

#include "Core.h"
#include "ScopeTimer.h"
#include "Serialization.h"

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(FunctionStats, 0) {
  ORBIT_NVP_VAL(0, m_Address);
  ORBIT_NVP_VAL(0, m_Count);
  ORBIT_NVP_VAL(0, m_TotalTimeMs);
  ORBIT_NVP_VAL(0, m_AverageTimeMs);
  ORBIT_NVP_VAL(0, m_MinMs);
  ORBIT_NVP_VAL(0, m_MaxMs);
}
