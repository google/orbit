// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessMemoryClient.h"

#include "ProcessMemoryService.h"

ProcessMemoryClient::ProcessMemoryClient(
    orbit::TransactionManager* transaction_manager)
    : transaction_manager_{transaction_manager} {
  auto on_response = [this](const Message& msg, uint32_t id) {
    HandleResponse(msg, id);
  };
  transaction_manager_->RegisterTransactionHandler(
      {nullptr, on_response, Msg_MemoryTransfer, "Memory Transfer"});
}

void ProcessMemoryClient::GetRemoteMemory(
    uint32_t pid, uint64_t address, uint64_t size,
    const ProcessMemoryCallback& callback) {
  ProcessMemoryRequest request{pid, address, size};
  uint32_t id =
      transaction_manager_->EnqueueRequest(Msg_MemoryTransfer, request);

  absl::MutexLock lock(&transaction_mutex_);
  callbacks_[id] = callback;
}

void ProcessMemoryClient::HandleResponse(const Message& message, uint32_t id) {
  std::vector<uint8_t> bytes;
  transaction_manager_->ReceiveResponse(message, &bytes);

  absl::MutexLock lock(&transaction_mutex_);
  callbacks_.at(id)(bytes);
  callbacks_.erase(id);
}
