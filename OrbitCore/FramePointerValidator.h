#ifndef ORBIT_CORE_FRAME_POINTER_VALIDATOR_H_
#define ORBIT_CORE_FRAME_POINTER_VALIDATOR_H_

#include <vector>

#include "OrbitFunction.h"

class FramePointerValidator {
 public:
  static std::vector<std::shared_ptr<Function>> GetFpoFunctions(
      std::vector<std::shared_ptr<Function>> functions, std::string file_name,
      bool is_64_bit);
};

#endif  // ORBIT_CORE_FRAME_POINTER_VALIDATOR_H_
