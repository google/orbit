//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "EventBuffer.h"

#include "Capture.h"
#include "Params.h"
#include "SamplingProfiler.h"
#include "Serialization.h"

#ifdef __linux
EventTracer GEventTracer;
#endif

//-----------------------------------------------------------------------------
void EventBuffer::Print() {
  PRINT("Orbit Callstack Events:");

  size_t numCallstacks = 0;
  for (auto& pair : m_CallstackEvents) {
    std::map<long long, CallstackEvent>& callstacks = pair.second;
    numCallstacks += callstacks.size();
  }

  PRINT_VAR(numCallstacks);

  for (auto& pair : m_CallstackEvents) {
    ThreadID threadID = pair.first;
    std::map<long long, CallstackEvent>& callstacks = pair.second;
    PRINT_VAR(threadID);
    PRINT_VAR(callstacks.size());
  }
}

//-----------------------------------------------------------------------------
std::vector<CallstackEvent> EventBuffer::GetCallstackEvents(
    long long a_TimeBegin, long long a_TimeEnd, ThreadID a_ThreadId /*= 0*/) {
  std::vector<CallstackEvent> callstackEvents;
  for (auto& pair : m_CallstackEvents) {
    ThreadID threadID = pair.first;
    std::map<long long, CallstackEvent>& callstacks = pair.second;

    if (a_ThreadId == 0 || threadID == a_ThreadId) {
      for (auto it = callstacks.lower_bound(a_TimeBegin);
           it != callstacks.end(); ++it) {
        long long time = it->first;
        if (time < a_TimeEnd) {
          callstackEvents.push_back(it->second);
        }
      }
    }
  }

  return callstackEvents;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(EventBuffer, 0) {
  ORBIT_NVP_VAL(0, m_CallstackEvents);

  long long maxTime = m_MaxTime;
  ORBIT_NVP_VAL(0, maxTime);
  m_MaxTime = maxTime;

  long long minTime = m_MinTime;
  ORBIT_NVP_VAL(0, minTime);
  m_MinTime = minTime;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(CallstackEvent, 0) {
  ORBIT_NVP_VAL(0, m_Time);
  ORBIT_NVP_VAL(0, m_Id);
  ORBIT_NVP_VAL(0, m_TID);
}

#ifdef __linux

//-----------------------------------------------------------------------------
void EventTracer::Start(uint32_t a_PID) {
  Capture::NewSamplingProfiler();
  Capture::GSamplingProfiler->StartCapture();

  if (GParams.m_SampleWithPerf) {
    m_Perf = std::make_shared<LinuxPerf>(a_PID);
    m_Perf->Start();
  }

  if (GParams.m_TrackContextSwitches || !GParams.m_SampleWithPerf) {
    m_EventTracer = std::make_shared<LinuxEventTracer>(a_PID);
    m_EventTracer->Start();
  }
}

//-----------------------------------------------------------------------------
void EventTracer::Stop() {
  if (m_Perf) {
    m_Perf->Stop();
  }

  if (Capture::GSamplingProfiler) {
    Capture::GSamplingProfiler->StopCapture();
    Capture::GSamplingProfiler->ProcessSamples();
  }

  if (m_EventTracer) {
    m_EventTracer->Stop();
  }
}

//-----------------------------------------------------------------------------
size_t EventBuffer::GetNumEvents() const {
  size_t numEvents = 0;
  for (auto& pair : m_CallstackEvents) {
    numEvents += pair.second.size();
  }

  return numEvents;
}

#endif
