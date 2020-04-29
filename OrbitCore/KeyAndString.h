#ifndef ORBIT_CORE_KEY_AND_STRING_H_
#define ORBIT_CORE_KEY_AND_STRING_H_

#include <string>
#include <utility>

#include "Serialization.h"
#include "SerializationMacros.h"

struct KeyAndString {
  KeyAndString() = default;
  KeyAndString(uint64_t key, std::string str) : key(key), str(std::move(str)) {}

  uint64_t key = 0;
  std::string str;

  ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE(KeyAndString, 0) {
  ORBIT_NVP_VAL(0, key);
  ORBIT_NVP_VAL(0, str);
}

#endif
