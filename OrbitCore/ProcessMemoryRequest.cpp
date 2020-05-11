#include "ProcessMemoryRequest.h"

#include "Serialization.h"

ORBIT_SERIALIZE(ProcessMemoryRequest, 0) {
  ORBIT_NVP_VAL(0, pid);
  ORBIT_NVP_VAL(0, address);
  ORBIT_NVP_VAL(0, size);
}
