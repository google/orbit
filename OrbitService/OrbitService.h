#pragma once

#include <vector>

#include "ProcessUtils.h"

class OrbitService {
 public:
  OrbitService();
  void Run();

 private:
  // TODO: where is this set?
  bool exit_requested_ = false;
};
