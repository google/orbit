#pragma once

#include <memory>
#include <vector>

#include "CoreApp.h"
#include "ProcessUtils.h"
#include "SymbolsService.h"

class OrbitService {
 public:
  explicit OrbitService(uint16_t port);
  void Run(std::atomic<bool>* exit_requested);

 private:
  void SetupServiceMemoryTransaction();

  std::unique_ptr<CoreApp> core_app_;
  std::unique_ptr<orbit::TransactionManager> transaction_manager_;
  std::unique_ptr<SymbolsService> symbols_service_;
};
