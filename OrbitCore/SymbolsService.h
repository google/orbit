// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SYMBOLS_SERVICE_H_
#define ORBIT_CORE_SYMBOLS_SERVICE_H_

#include "ProcessUtils.h"
#include "TransactionService.h"

class SymbolsService {
 public:
  SymbolsService(const ProcessList* process_list,
                 TransactionService* transaction_service);

  SymbolsService() = delete;
  SymbolsService(const SymbolsService&) = delete;
  SymbolsService& operator=(const SymbolsService&) = delete;
  SymbolsService(SymbolsService&&) = delete;
  SymbolsService& operator=(SymbolsService&&) = delete;

 private:
  void HandleRequest(const Message& message);

  const ProcessList* process_list_;
  TransactionService* transaction_service_;
};

#endif  // ORBIT_CORE_SYMBOLS_SERVICE_H_
