// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FunctionStats.h"

#include "Core.h"
#include "ScopeTimer.h"
#include "Serialization.h"

//-----------------------------------------------------------------------------
template <class T>
inline void UpdateMax(T& a_Max, T a_Value) {
  if (a_Value > a_Max) a_Max = a_Value;
}

//-----------------------------------------------------------------------------
template <class T>
inline void UpdateMin(T& a_Min, T a_Value) {
  if (a_Min == 0 || a_Value < a_Min) a_Min = a_Value;
}

//-----------------------------------------------------------------------------
void FunctionStats::Update(const Timer& a_Timer) {
  ++m_Count;
  double elapsedMillis = a_Timer.ElapsedMillis();
  m_TotalTimeMs += elapsedMillis;
  m_AverageTimeMs = m_TotalTimeMs / static_cast<double>(m_Count);
  UpdateMax(m_MaxMs, elapsedMillis);
  UpdateMin(m_MinMs, elapsedMillis);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(FunctionStats, 0) {
  ORBIT_NVP_VAL(0, m_Address);
  ORBIT_NVP_VAL(0, m_Count);
  ORBIT_NVP_VAL(0, m_TotalTimeMs);
  ORBIT_NVP_VAL(0, m_AverageTimeMs);
  ORBIT_NVP_VAL(0, m_MinMs);
  ORBIT_NVP_VAL(0, m_MaxMs);
}
