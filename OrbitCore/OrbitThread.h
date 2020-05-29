/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include "RingBuffer.h"
#include "ScopeTimer.h"

//-----------------------------------------------------------------------------
class Thread {
 public:
  Thread() : m_TID(0), m_Handle(0), m_Init(false), counter(0) {
    m_Usage.Fill(0.f);
  }

  float GetUsage();
  void UpdateUsage();

  DWORD m_TID;
  HANDLE m_Handle;
  bool m_Init;
  FILETIME m_LastUserTime;
  FILETIME m_LastKernTime;
  Timer m_UpdateThreadTimer;
  RingBuffer<float, 32> m_Usage;
  int counter;
};
