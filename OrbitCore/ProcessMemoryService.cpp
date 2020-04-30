// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessMemoryService.h"

#include "Serialization.h"

ORBIT_SERIALIZE(ProcessMemoryRequest, 0) {
  ORBIT_NVP_VAL(0, pid);
  ORBIT_NVP_VAL(0, address);
  ORBIT_NVP_VAL(0, size);
}

ProcessMemoryService::ProcessMemoryService(
    orbit::TransactionManager* transaction_manager)
    : transaction_manager_{transaction_manager} {
  auto on_request = [this](const Message& msg) { HandleRequest(msg); };
  transaction_manager_->RegisterTransactionHandler(
      {on_request, nullptr, Msg_MemoryTransfer, "Memory Transfer"});
}

void ProcessMemoryService::HandleRequest(const Message& message) {
  // Receive request.
  ProcessMemoryRequest request;
  transaction_manager_->ReceiveRequest(message, &request);

  // Read target process memory.
  std::vector<uint8_t> bytes(request.size);
  uint64_t num_bytes_read = 0;
  if (!ReadProcessMemory(request.pid, request.address, bytes.data(),
                         request.size, &num_bytes_read)) {
    ERROR("ReadProcessMemory error attempting to read %#lx", request.address);
  }
  bytes.resize(num_bytes_read);

  // Send response to the client.
  transaction_manager_->SendResponse(message.GetType(), bytes);
}
