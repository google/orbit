// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessMemoryService.h"

ProcessMemoryService::ProcessMemoryService(
    TransactionService* transaction_service)
    : transaction_service_{transaction_service} {
  auto on_request = [this](const Message& msg) { HandleRequest(msg); };
  transaction_service_->RegisterTransactionRequestHandler(
      {on_request, Msg_MemoryTransfer, "Memory Transfer"});
}

void ProcessMemoryService::HandleRequest(const Message& message) {
  // Receive request.
  ProcessMemoryRequest request;
  transaction_service_->ReceiveRequest(message, &request);

  // Read target process memory.
  std::vector<uint8_t> bytes(request.size);
  uint64_t num_bytes_read = 0;
  if (!ReadProcessMemory(request.pid, request.address, bytes.data(),
                         request.size, &num_bytes_read)) {
    ERROR("ReadProcessMemory error attempting to read %#lx", request.address);
  }
  bytes.resize(num_bytes_read);

  // Send response to the client.
  transaction_service_->SendResponse(message.GetType(), bytes);
}
