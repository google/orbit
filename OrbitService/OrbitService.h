#pragma once

#include <memory>
#include <vector>

#include "CoreApp.h"
#include "ProcessMemoryService.h"
#include "ProcessUtils.h"
#include "SymbolsService.h"
#include "TransactionService.h"

class OrbitService {
 public:
  explicit OrbitService(uint16_t port);
  void Run(std::atomic<bool>* exit_requested);

 private:
  std::unique_ptr<TransactionService> transaction_service_;
  std::unique_ptr<SymbolsService> symbols_service_;
  std::unique_ptr<ProcessMemoryService> process_memory_service_;
};
