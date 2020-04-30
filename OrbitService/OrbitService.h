#pragma once

#include <memory>
#include <vector>

#include "CoreApp.h"
#include "ProcessUtils.h"

class OrbitService {
 public:
  explicit OrbitService(uint16_t port);
  void Run(std::atomic<bool>* exit_requested);

 private:
  std::unique_ptr<CoreApp> core_app_;
};
