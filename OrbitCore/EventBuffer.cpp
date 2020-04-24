//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "EventBuffer.h"

#include "Capture.h"
#include "Params.h"
#include "SamplingProfiler.h"
#include "Serialization.h"

#ifdef __linux
#include "EventTracer.h"
#include "LinuxTracingHandler.h"
EventTracer GEventTracer;
#endif

//-----------------------------------------------------------------------------
void EventBuffer::Print() {
  LOG("Orbit Callstack Events:");

  size_t numCallstacks = 0;
  for (auto& pair : m_CallstackEvents) {
    std::map<uint64_t, CallstackEvent>& callstacks = pair.second;
    numCallstacks += callstacks.size();
  }

  PRINT_VAR(numCallstacks);

  for (auto& pair : m_CallstackEvents) {
    ThreadID threadID = pair.first;
    std::map<uint64_t, CallstackEvent>& callstacks = pair.second;
    PRINT_VAR(threadID);
    PRINT_VAR(callstacks.size());
  }
}

//-----------------------------------------------------------------------------
std::vector<CallstackEvent> EventBuffer::GetCallstackEvents(
    uint64_t a_TimeBegin, uint64_t a_TimeEnd, ThreadID a_ThreadId /*= 0*/) {
  std::vector<CallstackEvent> callstackEvents;
  for (auto& pair : m_CallstackEvents) {
    ThreadID threadID = pair.first;
    std::map<uint64_t, CallstackEvent>& callstacks = pair.second;

    if (a_ThreadId == 0 || threadID == a_ThreadId) {
      for (auto it = callstacks.lower_bound(a_TimeBegin);
           it != callstacks.end(); ++it) {
        uint64_t time = it->first;
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

  uint64_t maxTime = m_MaxTime;
  ORBIT_NVP_VAL(0, maxTime);
  m_MaxTime = maxTime;

  uint64_t minTime = m_MinTime;
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
void EventTracer::Start(uint32_t /*pid*/, LinuxTracingSession* session) {
  Capture::NewSamplingProfiler();
  Capture::GSamplingProfiler->StartCapture();

  m_LinuxTracer = std::make_shared<LinuxTracingHandler>(
      Capture::GSamplingProfiler.get(), session, Capture::GTargetProcess.get(),
      &Capture::GSelectedFunctionsMap, &Capture::GNumContextSwitches);
  m_LinuxTracer->Start();
}

//-----------------------------------------------------------------------------
void EventTracer::Stop() {
  if (m_LinuxTracer) {
    m_LinuxTracer->Stop();
  }

  if (Capture::GSamplingProfiler) {
    Capture::GSamplingProfiler->StopCapture();
    Capture::GSamplingProfiler->ProcessSamples();
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
