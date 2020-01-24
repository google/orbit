//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "Callstack.h"
#include "Core.h"
#include "OrbitFunction.h"
#include "ScopeTimer.h"

class BpfTrace {
 public:
  typedef std::function<void(const std::string& a_Data)> Callback;
  BpfTrace(Callback a_Callback = nullptr);

  void Start();
  void Stop();
  void Toggle() { IsRunning() ? Stop() : Start(); }
  bool IsRunning() const { return !m_ExitRequested; }
  void SetBpfScript(const std::string& a_Script) { m_Script = a_Script; }
  std::string GetBpfScript();

 protected:
  uint64_t ProcessString(const std::string& a_String);
  void CommandCallback(const std::string& a_Line);
  void CommandCallbackWithCallstacks(const std::string& a_Line);
  bool WriteBpfScript();
  static void RunPerfEventOpen(bool* a_ExitRequested);
  static void RunPerfEventOpenSingleBuffers(bool* a_ExitRequested);

 private:
  std::map<std::string, std::vector<Timer>> m_TimerStacks;
  std::unordered_map<uint64_t, std::string> m_StringMap;
  std::string m_BpfCommand;

  std::shared_ptr<std::thread> m_Thread;
  bool m_ExitRequested = true;
  uint32_t m_PID = 0;
  bool m_UsePerfEvents;
  Callback m_Callback;
  std::string m_Script;
  std::string m_ScriptFileName;
  CallStack m_CallStack;
  std::string m_LastThreadName;
};
