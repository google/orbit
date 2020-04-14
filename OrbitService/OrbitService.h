#pragma once

#include <memory>
#include <vector>

#include "CoreApp.h"
#include "ProcessUtils.h"

class OrbitService {
 public:
  OrbitService();
  void Run();

 private:
  // TODO: where is this set?
  bool exit_requested_ = false;
  std::unique_ptr<CoreApp> core_app_;
};
