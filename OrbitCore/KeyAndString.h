#ifndef ORBIT_CORE_KEY_AND_STRING_H_
#define ORBIT_CORE_KEY_AND_STRING_H_

#include <string>
#include "Serialization.h"
#include "SerializationMacros.h"

struct KeyAndString {
  uint64_t key;
  std::string str;
  
  ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE(KeyAndString, 0) {
  ORBIT_NVP_VAL(0, key);
  ORBIT_NVP_VAL(0, str);
}

#endif 
