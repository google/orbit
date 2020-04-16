//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#include "Serialization.h"
#include "SerializationMacros.h"

//-----------------------------------------------------------------------------
struct LinuxAddressInfo {
  uint64_t address = 0;
  std::string module_name;
  std::string function_name;
  uint64_t offset_in_function = 0;

  ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE(LinuxAddressInfo, 0) {
  ORBIT_NVP_VAL(0, module_name);
  ORBIT_NVP_VAL(0, function_name);
  ORBIT_NVP_VAL(0, address);
  ORBIT_NVP_VAL(0, offset_in_function);
}
