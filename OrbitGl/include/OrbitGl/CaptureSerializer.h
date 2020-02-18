//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <unordered_map>

#include "OrbitCore/OrbitType.h"
#include "OrbitCore/SerializationMacros.h"

//-----------------------------------------------------------------------------
class CaptureSerializer {
 public:
  CaptureSerializer();
  void Save(const std::wstring a_FileName);
  void Load(const std::wstring a_FileName);

  template <class T>
  void Save(T& a_Archive);

  class TimeGraph* m_TimeGraph;
  class SamplingProfiler* m_SamplingProfiler;

  std::string m_CaptureName;
  int m_Version;
  int m_TimerVersion;
  int m_NumTimers;
  int m_SizeOfTimer;

  ORBIT_SERIALIZABLE;
};
