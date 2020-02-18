//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitModule.h"
#include "OrbitType.h"

//-----------------------------------------------------------------------------
namespace google_breakpad {
class Minidump;
}
class Process;

//-----------------------------------------------------------------------------
class MiniDump {
 public:
  MiniDump(std::wstring a_FileName);
  ~MiniDump();

  std::shared_ptr<Process> ToOrbitProcess();

 protected:
  std::vector<Module> m_Modules;
  google_breakpad::Minidump* m_MiniDump;
};