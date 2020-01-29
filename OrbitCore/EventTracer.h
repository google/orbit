//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

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
};

extern EventTracer GEventTracer;

#endif