//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#include "Serialization.h"
#include "SerializationMacros.h"

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
