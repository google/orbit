// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_PROCESS_MEMORY_SERVICE_H_
#define ORBIT_CORE_PROCESS_MEMORY_SERVICE_H_

#include <cstdint>

#include "ProcessMemoryRequest.h"
#include "TransactionService.h"

class ProcessMemoryService {
 public:
  explicit ProcessMemoryService(TransactionService* transaction_service);

  ProcessMemoryService() = delete;
  ProcessMemoryService(const ProcessMemoryService&) = delete;
  ProcessMemoryService& operator=(const ProcessMemoryService&) = delete;
  ProcessMemoryService(ProcessMemoryService&&) = delete;
  ProcessMemoryService& operator=(ProcessMemoryService&&) = delete;

 private:
  void HandleRequest(const Message& message);

  TransactionService* transaction_service_;
};

#endif  // ORBIT_CORE_PROCESS_MEMORY_SERVICE_H_
