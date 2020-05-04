// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_PROCESS_MEMORY_SERVICE_H_
#define ORBIT_CORE_PROCESS_MEMORY_SERVICE_H_

#include <cstdint>

#include "SerializationMacros.h"
#include "TransactionService.h"

struct ProcessMemoryRequest {
  ProcessMemoryRequest() = default;
  ProcessMemoryRequest(uint32_t pid, uint64_t address, uint64_t size)
      : pid{pid}, address{address}, size{size} {}

  uint32_t pid = 0;
  uint64_t address = 0;
  uint64_t size = 0;

  ORBIT_SERIALIZABLE;
};

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
