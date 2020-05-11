// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessMemoryClient.h"

#include "ProcessMemoryRequest.h"

ProcessMemoryClient::ProcessMemoryClient(TransactionClient* transaction_client)
    : transaction_client_{transaction_client} {
  auto on_response = [this](const Message& msg, uint64_t id) {
    HandleResponse(msg, id);
  };
  transaction_client_->RegisterTransactionResponseHandler(
      {on_response, Msg_MemoryTransfer, "Memory Transfer"});
}

void ProcessMemoryClient::GetRemoteMemory(
    uint32_t pid, uint64_t address, uint64_t size,
    const CoreApp::ProcessMemoryCallback& callback) {
  ProcessMemoryRequest request{pid, address, size};
  uint64_t id =
      transaction_client_->EnqueueRequest(Msg_MemoryTransfer, request);

  absl::MutexLock lock(&transaction_mutex_);
  callbacks_[id] = callback;
}

void ProcessMemoryClient::HandleResponse(const Message& message, uint64_t id) {
  std::vector<uint8_t> bytes;
  transaction_client_->ReceiveResponse(message, &bytes);

  absl::MutexLock lock(&transaction_mutex_);
  callbacks_.at(id)(bytes);
  callbacks_.erase(id);
}
