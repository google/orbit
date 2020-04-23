//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ScopeTimer.h"

#include "Log.h"
#include "TimerManager.h"
#include "absl/strings/str_format.h"

thread_local size_t CurrentDepth = 0;
thread_local size_t CurrentDepthLocal = 0;

void Timer::Start() {
  m_TID = GetCurrentThreadId();
  m_Depth = CurrentDepth++;
  m_SessionID = Message::GSessionID;
  m_Start = OrbitTicks();
}

void Timer::Stop() {
  m_End = OrbitTicks();
  --CurrentDepth;
}

ScopeTimer::ScopeTimer(const char*) { m_Timer.Start(); }

ScopeTimer::~ScopeTimer() { m_Timer.Stop(); }

LocalScopeTimer::LocalScopeTimer() : millis_(nullptr) { ++CurrentDepthLocal; }

LocalScopeTimer::LocalScopeTimer(double* millis) : millis_(millis) {
  ++CurrentDepthLocal;
  timer_.Start();
}

LocalScopeTimer::LocalScopeTimer(const std::string& message)
    : millis_(nullptr), message_(message) {
  std::string tabs;
  for (size_t i = 0; i < CurrentDepthLocal; ++i) {
    tabs += "  ";
  }
  LOG("%sStarting %s...", tabs.c_str(), message_.c_str());

  ++CurrentDepthLocal;
  timer_.Start();
}

LocalScopeTimer::~LocalScopeTimer() {
  timer_.Stop();
  --CurrentDepthLocal;

  if (millis_ != nullptr) {
    *millis_ = timer_.ElapsedMillis();
  }

  if (!message_.empty()) {
    std::string tabs;
    for (size_t i = 0; i < CurrentDepthLocal; ++i) {
      tabs += "  ";
    }

    LOG("%s%s took %f ms.", tabs.c_str(), message_.c_str(),
        timer_.ElapsedMillis());
  }
}

void ConditionalScopeTimer::Start(const char*) {
  m_Timer.Start();
  m_Active = true;
}

ConditionalScopeTimer::~ConditionalScopeTimer() {
  if (m_Active) {
    m_Timer.Stop();
  }
}
