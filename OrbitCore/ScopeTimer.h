// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SCOPE_TIMER_H_
#define ORBIT_CORE_SCOPE_TIMER_H_

#include <string>

#include "Profiling.h"

#define SCOPE_TIMER_LOG(msg) LocalScopeTimer UNIQUE_ID(msg)
#define SCOPE_TIMER_FUNC SCOPE_TIMER_LOG(__FUNCTION__)

#pragma pack(push, 1)
class Timer {
 public:
  Timer()
      : m_PID(0),
        m_TID(0),
        m_Depth(0),
        m_Type(NONE),
        m_Processor(-1),
        m_CallstackHash(0),
        m_FunctionAddress(0) {
    m_UserData[0] = 0;
    m_UserData[1] = 0;
  }

  void Start();
  void Stop();
  void Reset() {
    Stop();
    Start();
  }

  inline double ElapsedMicros() const;
  inline double ElapsedMillis() const;
  inline double ElapsedSeconds() const;

  inline double QueryMillis() {
    Stop();
    return ElapsedMillis();
  }
  inline double QuerySeconds() {
    Stop();
    return ElapsedSeconds();
  }

  enum Type : uint8_t {
    NONE,
    CORE_ACTIVITY,
    INTROSPECTION,
    GPU_ACTIVITY,
  };

 public:
  // Needs to have to exact same layout in win32/x64, debug/release

  int32_t m_PID;
  int32_t m_TID;
  uint8_t m_Depth;
  Type m_Type;
  uint8_t m_Processor;
  uint64_t m_CallstackHash;
  uint64_t m_FunctionAddress;
  uint64_t m_UserData[2];
  TickType m_Start;
  TickType m_End;
};
#pragma pack(pop)

class ScopeTimer {
 public:
  ScopeTimer() {}
  ScopeTimer(const char* a_Name);
  ~ScopeTimer();

 protected:
  Timer m_Timer;
};

class LocalScopeTimer {
 public:
  LocalScopeTimer();
  LocalScopeTimer(const std::string& message);
  LocalScopeTimer(double* millis);
  ~LocalScopeTimer();

 protected:
  Timer timer_;
  double* millis_;
  std::string message_;
};

class ConditionalScopeTimer {
 public:
  ConditionalScopeTimer() : m_Active(false) {}
  ~ConditionalScopeTimer();
  void Start(const char* a_Name);

 protected:
  enum { NameSize = 64 };

  Timer m_Timer;
  bool m_Active;
  char m_Name[NameSize];
};

inline double Timer::ElapsedMicros() const {
  return TicksToMicroseconds(m_Start, m_End);
}

inline double Timer::ElapsedMillis() const { return ElapsedMicros() * 0.001; }

inline double Timer::ElapsedSeconds() const {
  return ElapsedMicros() * 0.000001;
}
#endif  // ORBIT_CORE_SCOPE_TIMER_H_
