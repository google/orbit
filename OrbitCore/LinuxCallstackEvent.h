//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include <string>

#include "Callstack.h"
#include "SerializationMacros.h"

//-----------------------------------------------------------------------------
class LinuxCallstackEvent {
 public:
  std::string m_header = "";
  uint64_t m_time = 0;
  uint64_t m_numCallstacks = 0;

  CallStack m_CS;

  void Clear();

  ORBIT_SERIALIZABLE;
};