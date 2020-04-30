// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SYMBOLS_SERVICE_H_
#define ORBIT_CORE_SYMBOLS_SERVICE_H_

#include "ProcessUtils.h"
#include "TransactionManager.h"

class SymbolsService {
 public:
  SymbolsService(const ProcessList* process_list,
                 orbit::TransactionManager* transaction_manager);

  SymbolsService() = delete;
  SymbolsService(const SymbolsService&) = delete;
  SymbolsService& operator=(const SymbolsService&) = delete;
  SymbolsService(SymbolsService&&) = delete;
  SymbolsService& operator=(SymbolsService&&) = delete;

 private:
  void HandleRequest(const Message& message);

  const ProcessList* process_list_;
  orbit::TransactionManager* transaction_manager_;
};

#endif  // ORBIT_CORE_SYMBOLS_SERVICE_H_
