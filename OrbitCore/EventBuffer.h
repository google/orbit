//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BlockChain.h"
#include "Callstack.h"
#include "Core.h"
#include "SerializationMacros.h"

#ifdef __linux
#include "LinuxTracingHandler.h"
#include "LinuxUtils.h"
#endif

#include <set>

//-----------------------------------------------------------------------------
struct CallstackEvent {
  CallstackEvent(long long a_Time = 0, CallstackID a_Id = 0, ThreadID a_TID = 0)
      : m_Time(a_Time), m_Id(a_Id), m_TID(a_TID) {}
  long long m_Time;
  CallstackID m_Id;
  ThreadID m_TID;

  ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
class EventBuffer {
 public:
  EventBuffer() : m_MaxTime(0), m_MinTime(LLONG_MAX) {}
  ~EventBuffer() {}

  void Print();
  void Reset() {
    m_CallstackEvents.clear();
    m_MinTime = LLONG_MAX;
    m_MaxTime = 0;
  }
  std::map<ThreadID, std::map<long long, CallstackEvent> >& GetCallstacks() {
    return m_CallstackEvents;
  }
  Mutex& GetMutex() { return m_Mutex; }
  std::vector<CallstackEvent> GetCallstackEvents(long long a_TimeBegin,
                                                 long long a_TimeEnd,
                                                 ThreadID a_ThreadId = 0);
  long long GetMaxTime() const { return m_MaxTime; }
  long long GetMinTime() const { return m_MinTime; }
  bool HasEvent() {
    ScopeLock lock(m_Mutex);
    return m_CallstackEvents.size() > 0;
  }
  bool HasEvent(ThreadID a_TID) {
    ScopeLock lock(m_Mutex);
    return m_CallstackEvents.find(a_TID) != m_CallstackEvents.end();
  }

#ifdef __linux__
  size_t GetNumEvents() const;
#endif

  //-----------------------------------------------------------------------------
  void RegisterTime(long long a_Time) {
    if (a_Time > m_MaxTime) m_MaxTime = a_Time;
    if (a_Time > 0 && a_Time < m_MinTime) m_MinTime = a_Time;
  }

  //-----------------------------------------------------------------------------
  void AddCallstackEvent(long long a_Time, CallstackID a_CSHash,
                         ThreadID a_TID) {
    ScopeLock lock(m_Mutex);
    std::map<long long, CallstackEvent>& threadMap = m_CallstackEvents[a_TID];
    threadMap[a_Time] = CallstackEvent(a_Time, a_CSHash, a_TID);
    RegisterTime(a_Time);
  }

  ORBIT_SERIALIZABLE;

 private:
  Mutex m_Mutex;
  std::map<ThreadID, std::map<long long, CallstackEvent> > m_CallstackEvents;
  std::atomic<long long> m_MaxTime;
  std::atomic<long long> m_MinTime;
};

#ifdef __linux
class EventTracer {
 public:
  EventBuffer& GetEventBuffer() { return m_EventBuffer; }
  EventBuffer m_EventBuffer;
  void Start(uint32_t a_PID);
  void Stop();

 private:
  std::shared_ptr<LinuxPerf> m_Perf;
  std::shared_ptr<LinuxTracingHandler> m_LinuxTracer;
};

extern EventTracer GEventTracer;
#endif
