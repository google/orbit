//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "Serialization.h"

//-----------------------------------------------------------------------------
struct SessionModule {
  std::string m_Name;
  std::vector<uint64_t> m_FunctionHashes;

  ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
class Session {
 public:
  Session();
  ~Session();

  ORBIT_SERIALIZABLE;

  std::string m_FileName;
  std::string m_ProcessFullPath;
  std::string m_WorkingDirectory;
  std::string m_Arguments;
  std::map<std::string, SessionModule> m_Modules;
};
