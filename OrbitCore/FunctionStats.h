// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "BaseTypes.h"
#include "SerializationMacros.h"

//-----------------------------------------------------------------------------
struct FunctionStats {
  FunctionStats() { Reset(); }
  void Reset() { memset(this, 0, sizeof(*this)); }

  uint64_t m_Count;
  double m_TotalTimeMs;
  double m_AverageTimeMs;
  double m_MinMs;
  double m_MaxMs;

  ORBIT_SERIALIZABLE;
};