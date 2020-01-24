//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "BaseTypes.h"
#include "LinuxCallstackEvent.h"
#include "Serialization.h"
#include "SerializationMacros.h"

//-----------------------------------------------------------------------------
class LinuxPerf {
 public:
  LinuxPerf(uint32_t a_PID, uint32_t a_Freq = 1000);
  void Start();
  void Stop();
  bool IsRunning() const { return !m_ExitRequested; }
  void LoadPerfData(std::istream& a_Stream);
  void HandleLine(const std::string& a_Line);

 private:
  uint32_t m_PID = 0;
  uint32_t m_ForkedPID = 0;
  uint32_t m_Frequency = 1000;

  std::shared_ptr<std::thread> m_Thread;
  bool m_ExitRequested = true;

  std::function<void(const std::string& a_Data)> m_Callback;
  std::string m_PerfCommand;

  LinuxCallstackEvent m_PerfData;
};

//-----------------------------------------------------------------------------
struct LinuxSymbol {
  std::string m_Module;
  std::string m_Name;
  std::string m_File;
  uint32_t m_Line = 0;
  uint64_t m_Address = 0;

  ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE(LinuxSymbol, 0) {
  ORBIT_NVP_VAL(0, m_Module);
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_File);
  ORBIT_NVP_VAL(0, m_Line);
  ORBIT_NVP_VAL(0, m_Address);
}