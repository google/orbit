/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include <thread>

#ifdef _WIN32

#include <evntrace.h>

#include "Core.h"
#include "EventBuffer.h"
#include "EventGuid.h"

//-----------------------------------------------------------------------------
class EventTracer {
 public:
  EventTracer();
  ~EventTracer();

  void Init();
  void Start();
  void Stop();

  void CleanupTrace();
  void EventTracerThread();
  void SetSamplingFrequency(float a_Frequency = 1000.f /*Hertz*/);
  void SetupStackTracing();
  bool IsTracing() { return m_IsTracing; }

  EventBuffer& GetEventBuffer() { return m_EventBuffer; }

 protected:
  ULONG64 m_SessionHandle;
  ULONG64 m_TraceHandle;
  std::atomic<bool> m_IsTracing;
  _EVENT_TRACE_PROPERTIES* m_SessionProperties;
  EventBuffer m_EventBuffer;

 private:
  std::thread thread_;
};

extern EventTracer GEventTracer;

#elif defined(__linux)
#include "LinuxTracingHandler.h"

class EventTracer {
 public:
  EventBuffer& GetEventBuffer() { return m_EventBuffer; }

 private:
  EventBuffer m_EventBuffer;
};

extern EventTracer GEventTracer;
#endif
