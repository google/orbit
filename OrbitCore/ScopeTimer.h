// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SCOPE_TIMER_H_
#define ORBIT_CORE_SCOPE_TIMER_H_

#include <string>

#include "Profiling.h"

#define SCOPE_TIMER_LOG(msg) LocalScopeTimer UNIQUE_ID(msg)

#pragma pack(push, 1)
class Timer {
 public:
  Timer() = default;

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

  int32_t m_PID = 0;
  int32_t m_TID = 0;
  uint8_t m_Depth = 0;
  Type m_Type = NONE;
  uint8_t m_Processor = -1;
  uint64_t m_CallstackHash = 0;
  uint64_t m_FunctionAddress = 0;
  uint64_t m_UserData[2] = {0};
  uint64_t m_Registers[6] = {0};
  TickType m_Start = 0;
  TickType m_End = 0;
};
#pragma pack(pop)

class ScopeTimer {
 public:
  ScopeTimer() = default;
  explicit ScopeTimer(const char* a_Name);
  ~ScopeTimer();

 protected:
  Timer m_Timer;
};

class LocalScopeTimer {
 public:
  LocalScopeTimer();
  explicit LocalScopeTimer(const std::string& message);
  explicit LocalScopeTimer(double* millis);
  ~LocalScopeTimer();

 protected:
  Timer timer_;
  double* millis_;
  std::string message_;
};

inline double Timer::ElapsedMicros() const { return TicksToMicroseconds(m_Start, m_End); }

inline double Timer::ElapsedMillis() const { return ElapsedMicros() * 0.001; }

inline double Timer::ElapsedSeconds() const { return ElapsedMicros() * 0.000001; }

#endif  // ORBIT_CORE_SCOPE_TIMER_H_
